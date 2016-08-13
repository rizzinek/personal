<?php
$path_pref = "/home2/thelosu5/keys/";
$priv_file = $path_pref."prv";
$pub_file = $path_pref."pbl";
$client_pub_file = $path_pref."client_pbl";
$cipher = $_POST["cipher"];
#$cipher = "1 2";

$dbUser = "thelosu5_keys";
$dbPass = "Q1w2e3r4";
$dbHost = "localhost";
$dbName = "thelosu5_useraccs";
$dbTable = "useraccs";
$dbDefaultMaxInstalls = 5;

$dbEmailCol = "email";
$dbKeyCol = "cdkey";
$dbMaxInstCol = "maxInstalls";
$dbIDCol = "deviceIDs";

$cdKeyPositive = "PRAVDA";
$cdKeyNegative = "LOJ";

if(strlen($cipher) == 0)
  exit("Decoder : empty cipher!\n");
if(!file_exists($priv_file) || !file_exists($pub_file))
  exit("Decoder : file doesn\'t exist!\n");

$private_key = file_get_contents($priv_file);
$public_key = file_get_contents($pub_file);
$p = 1;
$g = 1;
$y = 1;

if(strlen($private_key) == 0 || strlen($public_key) == 0)
  exit("Decoder : keys are messed up!\n");

$pk = explode(" ", $public_key);
if(count($pk) >= 3)
{
  $p = gmp_init($pk[0]);
  $g = gmp_init($pk[1]);
  $y = gmp_init($pk[2]);
}

$ints = explode(" ", $cipher);
if(count($ints) < 2)
  exit("Decoder : cipher has less than two entries.\n");
$a = gmp_init($ints[0]);
$b = gmp_init($ints[1]);
#echo "a = ".gmp_strval($a)."\nb = ".gmp_strval($b)."\np = ".gmp_strval($p)."\n".gmp_strval($private_key)."\n";

$power = gmp_sub(gmp_sub($p, $private_key), "1");
$a = gmp_powm($a, $power, $p);
$b = gmp_mod($b, $p);
#echo "power = ".gmp_strval($power)."\nprivKey = ".gmp_strval($private_key)."\n";
#echo "a = ".gmp_strval($a)."\nb = ".gmp_strval($b)."\n";

$s = gmp_mod(gmp_mul($a, $b), $p);
#echo "s : ".gmp_strval($s)."\n";
  
$bits = gmp_strval($s, 2);
while(strlen($bits) % 8 != 0)
  $bits = "0".$bits;

#echo "bits : $bits\n";

$res = "";
$size = strlen($bits) / 8;
$codes = array();
for($i = 0; $i < $size; $i++)
{
  $num = 0;
  $base = 1;
  for($j = 7; $j >= 0; $j--)
  {
    $num += $bits[8 * $i + $j] == '1' ? $base : 0;
    $base *= 2;
  }

  $pmod = gmp_intval(gmp_mod($p, "13"));
  $codes[] = ($num + 256 - $pmod) % 256;
}

for($i = 0; $i < $size; $i++)
{
  $ch = mb_convert_encoding(pack("N", $codes[$i]), "UTF-8", "UCS-4BE");
  $res = $res.$ch;
}
$rcvData = explode(" ", $res);

$email = $rcvData[2];
$deviceID = $rcvData[1];
$cdkey = $rcvData[0];

$dbHandle = "";
try
{
  $dbHandle = new PDO("mysql:dbname=".$dbName.";host=".$dbHost, $dbUser, $dbPass);
}
catch(PDOException $e)
{
  die("Decoder : couldn\'t connect to the DB.\n");
}
$email = $dbHandle->quote($email);
$cdkey = $dbHandle->quote($cdkey);

$dbData = $dbHandle->query("SELECT $dbEmailCol, $dbKeyCol, $dbMaxInstCol, $dbIDCol from $dbTable
  WHERE $dbEmailCol = ".$email." AND $dbKeyCol = ".$cdkey);
$dbRow = $dbData->fetch(PDO::FETCH_ASSOC);

$cdKeyResponse = $cdKeyNegative;

if(strlen($dbRow[$dbEmailCol]) == 0)
{
  $changed = $dbHandle->exec("INSERT INTO $dbTable ($dbEmailCol, $dbKeyCol, $dbMaxInstCol, $dbIDCol)
    VALUES ($email, $cdkey, $dbDefaultMaxInstalls, ".$dbHandle->quote($deviceID).")");
  #echo "Cahnged rows : $changed\n";
  $cdKeyResponse = $cdKeyPositive;
}
else
{
  $regIDs = explode(" ", $dbRow[$dbIDCol]);
  $found = FALSE;
  for($i = 0; $i < count($regIDs); $i++)
  {
    $found = ($regIDs[$i] == $deviceID);
    #echo "hahah : $regIDs[$i]\n";
  }
  if($found)
    $cdKeyResponse = $cdKeyPositive;
  else
  {
    $maxInst = intval($dbRow[$dbMaxInstCol]);
    #echo "MaxInst : $maxInst\n";
    if(count($regIDs) < $maxInst)
    {
      $deviceID = $dbRow[$dbIDCol]." ".$deviceID;
      #echo "New dIC : $deviceID\n";
      $changed = $dbHandle->exec("UPDATE $dbTable SET $dbIDCol = ".$dbHandle->quote($deviceID)
        ." WHERE $dbEmailCol = ".$email." AND $dbKeyCol = ".$cdkey);
      #echo "Changed rows : $changed\n";
      $cdKeyResponse = $cdKeyPositive;
    }
    else $cdKeyResponse = $cdKeyNegative;
  }
}

if(!file_exists($client_pub_file))
  exit("Decoder : client keys are missing!");
$cpb = explode(" ", file_get_contents($client_pub_file));
$cp = gmp_init($cpb[0]);
$cg = gmp_init($cpb[1]);
$cy = gmp_init($cpb[2]);

$cdKeyResponse = mb_convert_encoding($cdKeyResponse, "UTF-8");
$cdKeyCode = "";
for($i = 0; $i < strlen($cdKeyResponse); $i++)
{
  $charCode = unpack("N", mb_convert_encoding($cdKeyResponse[$i], "UCS-4BE", "UTF-8"));
  $cc = decbin(reset($charCode));
  while(strlen($cc) < 8)
    $cc = "0".$cc;
  $cdKeyCode = $cdKeyCode.$cc;
}
$cdKeyCode = gmp_init($cdKeyCode, 2);
$cdKeyCodeK = gmp_add(gmp_random(31), "2");
$cdKeyCodeA = gmp_powm($cg, $cdKeyCodeK, $cp);
$cdKeyCodeB = gmp_mod(gmp_mul(gmp_powm($cy, $cdKeyCodeK, $cp), $cdKeyCode),$cp);
echo gmp_strval($cdKeyCodeA)." ".gmp_strval($cdKeyCodeB);
?>
