using UnityEngine;
using UnityEngine.SceneManagement;
using System.Collections;
using System.Collections.Generic;
using UnityStandardAssets.CrossPlatformInput;

//I use scene names as identifiers
public class SceneLoader : MonoBehaviour
{
	public static readonly string coreSceneName = "core";
	public static readonly string mainMenuSceneName = "MainMenu";

	private Dictionary<string, List<string>> _requiredScenesLists = new Dictionary<string, List<string>>();
	private Dictionary<string, int> _sceneRequirementCounter = new Dictionary<string, int>();

	private Queue<string> _loadingQueue = new Queue<string>();
	private Coroutine _loaderCoroutine = null;

	public string lastLoadedLocation = string.Empty;

	void Awake()
	{
		if(_instance == null)
			_instance = this;
	}

	void Start()
	{
		Application.backgroundLoadingPriority = ThreadPriority.Low;
	}

	public static void LoadMainMenu()
	{
		SceneManager.LoadScene(mainMenuSceneName, LoadSceneMode.Single);
	}

	public void OpenScene(string sceneName)
	{
		SceneManager.LoadScene(sceneName, LoadSceneMode.Single);
	}

	public IEnumerator LoadCore(bool autoStart)
	{
		Application.backgroundLoadingPriority = ThreadPriority.High;
		yield return SceneManager.LoadSceneAsync(coreSceneName, LoadSceneMode.Single);
		Application.backgroundLoadingPriority = ThreadPriority.Low;

		SceneContainerCore sceneContainer = GetSceneContainer(coreSceneName) as SceneContainerCore;
		if(sceneContainer == null)
			throw new System.Exception("core scene: no SceneContainerCore found!");

		//for all the initialization in Start()
		yield return null;

		if(!autoStart)
			yield break;

		LoadLocation loader = sceneContainer.loadStartLocation;
		loader.StartItem();
	}

	//atm this is used only in NextSceneTrigger, so the use case of this function is:
	//player moves from location A to location B smoothly. There's no additional player setup
	//involved, location B gets loaded, location A unloaded
	public void LoadLocation(string nextLocation, string previousLocation)
	{
		StartCoroutine(LoadLocation_cor(nextLocation, previousLocation, true));
	}

	public IEnumerator LoadLocation_cor(string nextLocation, string previousLocation, bool autoUnload)
	{
		Debug.Log(previousLocation + " -> " + nextLocation);

		Application.backgroundLoadingPriority = ThreadPriority.High;
		yield return StartCoroutine(LoadScene_cor(nextLocation, true));
		Application.backgroundLoadingPriority = ThreadPriority.Low;

		if(autoUnload)
		{
			if(previousLocation == null || previousLocation.Length == 0)
				previousLocation = lastLoadedLocation;

			if(previousLocation != null && previousLocation.Length > 0)
				yield return StartCoroutine(UnloadScene_cor(previousLocation));
		}

		lastLoadedLocation = nextLocation;
	}

	public void UnloadLocationManual(string location)
	{
		StartCoroutine(UnloadScene_cor(location));
	}

	public SceneContainer GetSceneContainer(string sceneName)
	{
		Scene scene = SceneManager.GetSceneByName(sceneName);
		if(!scene.isLoaded)
			return null;
		foreach(var go in scene.GetRootGameObjects())
		{
			SceneContainer sceneCont = go.GetComponent<SceneContainer>();
			if(sceneCont != null)
				return sceneCont;
		}
		return null;
	}

	/* *
	 * PRIVATE BLOCK
	 * */

	private IEnumerator LoadScene_cor(string name, bool withPreload)
	{
		Scene existingScene = SceneManager.GetSceneByName(name);
		if(!existingScene.isLoaded)
		{
			//by design we don't need non-additive scene loading
			AsyncOperation loader = SceneManager.LoadSceneAsync(name, LoadSceneMode.Additive);
			loader.allowSceneActivation = true;

			yield return loader;
			existingScene = SceneManager.GetSceneByName(name);

			//remember what scenes does this scene require
			//there should be a single game object at the root of the scene
			SceneContainer container = GetSceneContainer(name);
			_requiredScenesLists.Add(name, new List<string>(container.scenesToPreload));
			_sceneRequirementCounter.Add(name, 0);

			//disable temporary starters
			#if UNITY_EDITOR
			GameObject[] objects = existingScene.GetRootGameObjects();
			string comparisonString = "temporary";
			foreach(var obj in objects)
			{
				if(obj.name.ToLower().IndexOf(comparisonString) != -1)
					obj.SetActive(false);
			}
			for(int i = 0; i < container.transform.childCount; ++i)
			{
				Transform t = container.transform.GetChild(i);
				if(t.name.ToLower().IndexOf(comparisonString) != -1)
					t.gameObject.SetActive(false);
			}
			#endif
		}
			
		_sceneRequirementCounter[name]++;

		//preload the required scenes if necessary
		if(withPreload)
			PreloadRequiredScenes(name);
	}

	private void PreloadRequiredScenes(string name)
	{
		foreach(var scene in _requiredScenesLists[name])
		{
			if(SceneManager.GetSceneByName(scene).isLoaded)
			{
				++_sceneRequirementCounter[scene];
			}
			else if(!_loadingQueue.Contains(scene))
			{
				_loadingQueue.Enqueue(scene);
				if(_loaderCoroutine == null)
					_loaderCoroutine = StartCoroutine(Preloader_cor());
			}
		}
	}

	private IEnumerator UnloadScene_cor(string name)
	{
		//make a copy
		List<string> reqScenes = new List<string>(_requiredScenesLists[name]);
		UnloadSingleScene(name);

		foreach(var scene in reqScenes)
		{
			UnloadSingleScene(scene);
		}

		AsyncOperation unloader = Resources.UnloadUnusedAssets();
		yield return unloader;
	}

	private IEnumerator Preloader_cor()
	{
		while(_loadingQueue.Count > 0)
		{
			string scene = _loadingQueue.Dequeue();
			yield return StartCoroutine(LoadScene_cor(scene, false));
		}
		_loaderCoroutine = null;
	}

	private void UnloadSingleScene(string name)
	{
		--_sceneRequirementCounter[name];
		if(_sceneRequirementCounter[name] == 0)
		{
			SceneManager.UnloadScene(name);
			_sceneRequirementCounter.Remove(name);
			_requiredScenesLists.Remove(name);
		}
	}

	private static readonly string c_gameObjectName = "SceneLoader";
	private SceneLoader() {}
	private static SceneLoader _instance = null;

	public static SceneLoader Instance
	{
		get
		{
			if(_instance == null)
			{
				GameObject go = GameObject.Find(c_gameObjectName);
				if(go == null)
					go = new GameObject(c_gameObjectName);
				_instance = go.GetComponent<SceneLoader>();
				if(_instance == null)
					_instance = go.AddComponent<SceneLoader>();
			}
			return _instance;
		}
	}
}
