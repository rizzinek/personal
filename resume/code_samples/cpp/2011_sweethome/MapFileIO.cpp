#include "MapFileIO.h"

#include "Background.h"
#include "Creature.h"
#include "CreaturesManager.h"
#include "LittleMap.h"
#include "LittleByLittle/SampleMob.h"
#include "StubCreature.h"
#include <set>
#include <sstream>

//in main.cpp
extern wchar_t g_ContentPath[MAX_PATH];

CMapFileIO::CMapFileIO(LPCWSTR szFile):
m_pgetsrc(0)
{
	SetFileName(szFile);
}

CMapFileIO::~CMapFileIO()
{
	delete[] m_fname;
}

void CMapFileIO::GetContentPath() const
{
	GetFullPathName(L"content", MAX_PATH, g_ContentPath, 0);
}

void CMapFileIO::SetGetSourceHandle(LPWSTR (*pgetsrc)(LPWSTR szType, LPWSTR szName))
{
	m_pgetsrc = pgetsrc;
}

int CMapFileIO::LoadMap(CLevelMap &map) const
{
	return LoadMap(map, m_fname);
}

int CMapFileIO::LoadMap(CLevelMap &map, HANDLE hFile) const
{
	XML_MANAGER_HANDLE xml = 0;
	xml = CreateWorker();
	if(!xml)
		return 1;
	DWORD res = ReadFromFileHandle(xml, hFile);
	if(res != ERROR_SUCCESS) 
		return 1;
	CloseHandle(hFile);
	XML_NODE_HANDLE group;

	//dimensions
	group = RetrieveNode(xml, 0, L"dimensions");
	if(!group)
		return 1;
	LPWSTR mapwt = GetNodeAttr(xml, group, L"width");
	if(!mapwt)
		return 1;
	LPWSTR mapht = GetNodeAttr(xml, group, L"height");
	if(!mapht)
		return 1;
	int mapw = _wtoi(mapwt);
	int maph = _wtoi(mapht);
	map.SetDimensions(mapw, maph);
	ReleaseTextString(mapwt);
	ReleaseTextString(mapht);
	ReleaseNode(group);

	//background
	group = RetrieveNode(xml, 0, L"background");
	if(!group)
		return 1;
	map.SetBackground(LoadBackground(xml, group));
	ReleaseNode(group);

	//player
	group = RetrieveNode(xml, 0, L"player");
	CStubCreature* pPlr = 0;
	if(group)
	{
		pPlr = LoadPlayer(xml, group);
		map.SetPlayerPos(pPlr->GetPosition());
		delete pPlr;
	}
	ReleaseNode(group);

	//tiletypes
	LoadTileTypesDict(map.m_TilesDictionary);

	//tiles
	group = RetrieveNode(xml, 0, L"tiles");
	if(!group)
		return 1;
	LoadTileList(map.m_pTiles, map.m_TilesDictionary, xml, group);
	ReleaseNode(group);

	//creatures
	group = RetrieveNode(xml, 0, L"creatures");
	if(group)
		LoadCreaturesList( map.m_pCreatures, xml, group );
	ReleaseNode(group);
	map.AssignMyEnviroment();

	ReleaseWorker(xml);
	return 0;
}

int CMapFileIO::LoadMap(CLevelMap &map, LPCWSTR szFileName) const
{
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, 0,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return 1;
	return LoadMap(map, hFile);
}

CBackground* CMapFileIO::LoadBackground(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode) const
{
	CBackground* pBg = 0;

	XML_NODE_HANDLE node = RetrieveNode(xml, hNode, L"image");
	if(!node)
		return 0;
	LPWSTR bgimage = GetNodeAttr(xml, node, L"file");
	ReleaseNode(node);
	node = RetrieveNode(xml, hNode, L"position");
	if(!node)
		return 0;
	LPWSTR bgxt = GetNodeAttr(xml, node, L"x");
	if(!bgxt)
		return 0;
	LPWSTR bgyt = GetNodeAttr(xml, node, L"y");
	if(!bgyt)
		return 0;
	LPWSTR bgzt = GetNodeAttr(xml, node, L"z");
	if(!bgzt)
		return 0;
	float x = (float)_wtof(bgxt);
	float y = (float)_wtof(bgyt);
	float z = (float)_wtof(bgzt);
	D3DXVECTOR3 pos = D3DXVECTOR3(x, y, z);

	//deleted at LittleMap.cpp
	pBg = new CBackground(bgimage, pos);

	ReleaseTextString(bgimage);
	ReleaseTextString(bgxt);
	ReleaseTextString(bgyt);
	ReleaseTextString(bgzt);
	ReleaseNode(node);

	return pBg;
}

CStubCreature* CMapFileIO::LoadPlayer(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hPlrNode) const
{
	CStubCreature *pPlr = 0;
	XML_NODE_HANDLE sub = RetrieveNode(xml, hPlrNode, L"position");
	if(!sub)
		return 0;
	LPWSTR xt = GetNodeAttr(xml, sub, L"x");
	if(!xt)
		return 0;
	LPWSTR yt = GetNodeAttr(xml, sub, L"y");
	if(!yt)
		return 0;
	LPWSTR zt = GetNodeAttr(xml, sub, L"z");
	if(!zt)
		return 0;
	float x = (float)_wtof(xt);
	float y = (float)_wtof(yt);
	float z = (float)_wtof(zt);
	D3DXVECTOR3 pos = D3DXVECTOR3(x, y, z);

	//deleted at LoadMap
	pPlr = new CStubCreature(0, 0, 0, 0, 0, pos, L"player", 0);

	ReleaseNode(sub);
	ReleaseTextString(xt);
	ReleaseTextString(yt);
	ReleaseTextString(zt);

	return pPlr;
}

ICreature* CMapFileIO::LoadCreature(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hCreNode) const
{
	ICreature* pCrt = 0;
	CSampleMobAnimator* pMa = 0;

	LPWSTR type = GetNodeAttr(xml, hCreNode, L"type");
	if(!type)
		return 0;
	LPWSTR name = GetNodeAttr(xml, hCreNode, L"name");
	if(!name)
		return 0;
	LPWSTR src = GetNodeAttr(xml, hCreNode, L"source");
	if(!src)
		return 0;

	//deleted at LittleMap.cpp
	pCrt = CCreatureManager::GetInstance().CreateCreature(type, name, src);
	_ASSERT( pCrt!=NULL );
	if( !pCrt )
		return NULL;

	//deleted at Creature's destructor
	pMa = CCreatureManager::GetInstance().CreateAnimator(type, name, src);
	pCrt->SetAnimatorRef(pMa);
	pCrt->SetName(name);

	//position
	XML_NODE_HANDLE sub = RetrieveNode(xml, hCreNode, L"Location");
	if(!sub)
		return 0;
	XML_NODE_HANDLE sub1 = RetrieveNode(xml, sub, L"position");
	LPWSTR xt = GetNodeAttr(xml, sub1, L"x");
	if(!xt)
		return 0;
	LPWSTR yt = GetNodeAttr(xml, sub1, L"y");
	if(!yt)
		return 0;
	LPWSTR zt = GetNodeAttr(xml, sub1, L"z");
	if(!zt)
		return 0;
	float x = (float)_wtof(xt);
	float y = (float)_wtof(yt);
	float z = (float)_wtof(zt);
	D3DXVECTOR3 pos = D3DXVECTOR3(x, y, z);

	pCrt->SetPosition(pos);

	ReleaseNode(sub);
	ReleaseNode(sub1);
	ReleaseTextString(name);
	ReleaseTextString(type);
	ReleaseTextString(src);
	ReleaseTextString(xt);
	ReleaseTextString(yt);
	ReleaseTextString(zt);

	return pCrt;
}

int CMapFileIO::LoadCreaturesList( std::list<ICreature*> &Creats, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode ) const
{
	XML_NODE_HANDLE node = RetrieveNode(xml, hNode, L"Monster");
	if(!node)
		return 1;

	while(node)
	{
		ICreature *pNewCreature = LoadCreature( xml, node );
		if( pNewCreature )
			Creats.push_back( pNewCreature );
		node = GetNextNode(xml, node);
	}
	return 0;
}

CTile* CMapFileIO::LoadTile(CTileTypesDict& Dict, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hTileNode) const
{
	CTile* t = 0;
	//name
	LPWSTR name = GetNodeAttr(xml, hTileNode, L"type");
	if(!name)
		return 0;
	//hp
	LPWSTR hpt = GetNodeAttr(xml, hTileNode, L"hp");
	if(!hpt)
		return 0;
	int hp = _wtoi(hpt);
	//position
	XML_NODE_HANDLE hPos = RetrieveNode(xml, hTileNode, L"position");
	if(!hPos)
		return 0;
	LPWSTR xt = GetNodeAttr(xml, hPos, L"x");
	if(!xt)
		return 0;
	LPWSTR yt = GetNodeAttr(xml, hPos, L"y");
	if(!yt)
		return 0;
	LPWSTR zt = GetNodeAttr(xml, hPos, L"z");
	if(!zt)
		return 0;
	float x = (float)_wtof(xt);
	float y = (float)_wtof(yt);
	float z = (float)_wtof(zt);

	CTileType* type = Dict.Give(name, false);
	if(type)
		//deleted at LittleMap.cpp
		t = new CTile(*type, x, y, z);

	ReleaseNode(hPos);
	ReleaseTextString(name);
	ReleaseTextString(hpt);
	ReleaseTextString(xt);
	ReleaseTextString(yt);
	ReleaseTextString(zt);

	return t;
}

int CMapFileIO::LoadTileList(std::list<CTile*>& List, CTileTypesDict& Dict,
							 XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode) const
{
	XML_NODE_HANDLE node = RetrieveNode(xml, hNode, L"tile");
	if(!node)
		return 1;
	while(node)
	{
		List.push_back(LoadTile(Dict, xml, node));
		node = GetNextNode(xml, node);
	}
	return 0;
}

int CMapFileIO::LoadTileTypesDict(CTileTypesDict& Dict) const
{
	WCHAR* fname = new WCHAR[MAX_PATH];
	wsprintf(fname, L"%s\\tiles\\*", g_ContentPath);
	WIN32_FIND_DATA ffd;

	XML_MANAGER_HANDLE xml = 0;
	HANDLE file = INVALID_HANDLE_VALUE;
	WCHAR* listpath = new WCHAR[MAX_PATH];
	WCHAR* imgpath = new WCHAR[MAX_PATH];
	CTileType *t;

	HANDLE search = FindFirstFile(fname, &ffd);
	do
	{
		if((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ffd.cFileName[0] != '.')
		{
			wsprintf(listpath, L"%s\\tiles\\%s\\tiles.xml", g_ContentPath, ffd.cFileName);
			file = INVALID_HANDLE_VALUE;
			file = CreateFile(listpath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if(file == INVALID_HANDLE_VALUE)
				return 1;   
			xml = 0;
			xml = CreateWorker();
			if(!xml)
				return 1;
			DWORD res = ReadFromFileHandle(xml, file);
			if(res != ERROR_SUCCESS) 
				return 1;
			CloseHandle(file);
			XML_NODE_HANDLE node = RetrieveNode(xml, 0, L"tile");
			while(node)
			{
				//name
				LPWSTR name = GetNodeAttr(xml, node, L"name");
				XML_NODE_HANDLE hImg = RetrieveNode(xml, node, L"img");
				XML_NODE_HANDLE hSt = RetrieveNode(xml, node, L"stats");
				XML_NODE_HANDLE hMov = RetrieveNode(xml, node, L"movement");
				//img
				LPWSTR img = GetNodeAttr(xml, hImg, L"path");
				//hp
				LPWSTR hpt = GetNodeAttr(xml, hSt, L"hp");
				int hp = _wtoi(hpt);
				//hrd
				LPWSTR hrdt = GetNodeAttr(xml, hSt, L"hardness");
				int hrd = _wtoi(hrdt);
				LPWSTR rgnt = GetNodeAttr(xml, hSt, L"regenspeed");
				float regenval = (float)_wtof(rgnt);

				LPWSTR blok = GetNodeAttr(xml, hMov, L"blocks");
				bool block = _wcsicmp(blok, L"YES") == 0;

				t = Dict.Give(name);
				t->SetHardness(hrd);
				t->SetMaxHP(hp);
				t->SetRegen(regenval);
				wsprintf(imgpath, L"tiles\\%s\\%s", ffd.cFileName, img);
				t->SetImageFile(imgpath);
				t->SetType(ffd.cFileName);

				ReleaseNode(hImg);
				ReleaseNode(hSt);
				ReleaseNode(hMov);

				ReleaseTextString(name);
				ReleaseTextString(img);
				ReleaseTextString(hrdt);
				ReleaseTextString(hpt);
				ReleaseTextString(blok);

				node = GetNextNode(xml, node);
			}
		} 
	}while(FindNextFile(search, &ffd) != 0);
	ReleaseWorker(xml);
	FindClose(search);
	delete[] imgpath;
	delete[] fname;
	delete[] listpath;
	return 0;
}

int CMapFileIO::SaveMap(const CLevelMap &map) const
{
	return SaveMap(map, m_fname);
}

int CMapFileIO::SaveMap(const CLevelMap &map, HANDLE hFile) const
{
	XML_MANAGER_HANDLE xml = 0;
	std::wostringstream os;
	xml = CreateWorker();
	if(!xml)
		return 1;
	BeginDoc(xml, L"map");
	XML_NODE_HANDLE root = GetRootNode(xml);
	if(!root)
		return 1;
	XML_NODE_HANDLE node;

	int w, h;
	D3DXVECTOR3 pos;

	//dimensions
	node = AddNode(xml, root, L"dimensions");
	if(!node)
		return 1;
	w = map.GetWidth();
	h = map.GetHeight();
	os << w;
	SetNodeAttr(xml, node, L"width", os.str().c_str());
	os.str(L"");
	os << h;
	SetNodeAttr(xml, node, L"height", os.str().c_str());
	os.str(L"");
	ReleaseNode(node);

	//background
	SaveBackground(*map.GetBackground(), xml, root);

	//player
	SavePlayer( map.GetPlayer(), xml, root );

	//tiles
	SaveTileList(map.m_pTiles, xml, root);

	//creatures
	SaveCreatureList(map.m_pCreatures, xml, root);

	SaveToFileHandle(xml, hFile, true);
	CloseHandle(hFile);
	ReleaseWorker(xml);
	return 0;
}

int CMapFileIO::SaveMap(const CLevelMap &map, LPCWSTR szFileName) const
{
	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, 0,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return 1;
	return SaveMap(map, hFile);
}

int CMapFileIO::SaveBackground(const CBackground& Back, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE group = AddNode(xml, hRoot, L"background");
	if(!group)
		return 1;
	XML_NODE_HANDLE node = AddNode(xml, group, L"image");
	SetNodeAttr(xml, node, L"file", Back.GetImagePath());
	WritePosition(xml, group, Back.GetPosition());
	ReleaseNode(node);
	ReleaseNode(group);

	return 0;
}

int CMapFileIO::SavePlayer( const ICreature* pPlayer, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE node = AddNode(xml, hRoot, L"player");
	if( !node )
		return 1;

	if( pPlayer )
		if( WritePosition( xml, node, pPlayer->GetPosition() ) )
			return 1;

	ReleaseNode(node);

	return 0;
}

int CMapFileIO::SaveCreature( const ICreature* pCrt, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE sub;
	XML_NODE_HANDLE node = AddNode(xml, hRoot, L"Monster");
	if(!node)
		return 1;
	SetNodeAttr(xml, node, L"type", pCrt->GetType());
	SetNodeAttr(xml, node, L"name", pCrt->GetName());
	SetNodeAttr(xml, node, L"source", m_pgetsrc((LPWSTR)pCrt->GetType(), (LPWSTR)pCrt->GetName()));
	sub = AddNode(xml, node, L"location");
	if(!sub)
		return 1;
	if(WritePosition(xml, sub, pCrt->GetPosition()))
		return 1;
	ReleaseNode(sub);
	ReleaseNode(node);

	return 0;
}

int CMapFileIO::SaveCreatureList(const std::list<ICreature*>& pCreatures, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE group = AddNode(xml, hRoot, L"creatures");
	if(!group)
		return 1;

	for( std::list<ICreature*>::const_iterator i = pCreatures.begin(); i != pCreatures.end(); ++i )
		SaveCreature(*i, xml, group);

	ReleaseNode(group);

	return 0;
}

int CMapFileIO::SaveTile(const CTile& Tile, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE node = AddNode(xml, hRoot, L"tile");
	if(!node)
		return 1;
	SetNodeAttr(xml, node, L"type", Tile.GetTypeName());
	if(WritePosition(xml, node, Tile.GetPosition()))
		return 1;
	std::wostringstream os;
	os << Tile.GetCurrentHP();
	SetNodeAttr(xml, node, L"hp", os.str().c_str());
	os.str(L"");
	ReleaseNode(node);

	return 0;
}

int CMapFileIO::SaveTileList(const std::list<CTile*>& pTiles, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const
{
	XML_NODE_HANDLE group = AddNode(xml, hRoot, L"tiles");
	if(!group)
		return 1;
	for(std::list<CTile*>::const_iterator i = pTiles.begin(); i != pTiles.end(); ++i)
		SaveTile(*(*i), xml, group);

	ReleaseNode(group);

	return 0;
}

void CMapFileIO::SetFileName(LPCWSTR szFile)
{
	if(szFile)
	{
		int n = wcslen(szFile) + 1;
		m_fname = new wchar_t[n]; //deleted in the destructor
		wcscpy_s(m_fname, n, szFile);
	}
	else
		m_fname = 0;
}

int CMapFileIO::WritePosition(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE parent, const D3DXVECTOR3& pos) const
{
	std::wostringstream os;
	os << pos.x;
	XML_NODE_HANDLE node = AddNode(xml, parent, L"position");
	SetNodeAttr(xml, node, L"x", os.str().c_str());
	os.str(L"");
	os << pos.y;
	SetNodeAttr(xml, node, L"y", os.str().c_str());
	os.str(L"");
	os << pos.z;
	SetNodeAttr(xml, node, L"z", os.str().c_str());
	ReleaseNode(node);
	return 0;
}