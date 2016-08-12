using UnityEngine;
using System.Collections.Generic;

//StoryNodes show the structure of the main storyline and of all the routes.
//Simple visualization of a story tree looks like this:
//                  -> C3c -> C4c
//                 /
// root -> C1 -> C2 -> C3 -> C4 -> C5 -> C6 -> C7
//                             \
//                              -> C5a -> C6a -> C7a
//
//Basically, the main storyline chapters go in a straight line with occasional
//routes branching out from them. The routes don't merge back with the main tree
//or with other routes.
//
//Each node has only one parent, but can have multiple children
//
//Each story node contains an initializer item that should set the game variables
//to values correct for the corresponding chapter
public class StoryNode : MonoBehaviour
{
	public List<StoryNode> children = new List<StoryNode>();

	public ItemBase initializer = null;

	public string targetLocationName = string.Empty;
	public int targetSpawnPointIdx = 0;

	//this is hidden, because it is set automatically inside this script
	//and shouldn't be an eyesore to the people working in editor
	[HideInInspector]
	public StoryNode parent = null;

	void Awake()
	{
		foreach(var child in children)
			child.parent = this;
	}

	//this function is for applying only the current node's initializer
	//values to the game data
	//atm it is only a wrapper around StartItem, but, perhaps, a more complex
	//behaviour will be required in the future
	public void ApplyStoryNodeValuesSingle()
	{
		initializer.StartItem();
	}

	//constructs a path from root to the current node, e.g. root->C1->C2->C3c
	//and, following the whole path, apples each story node's values to game data
	//
	//end result: the game data variables have the same values that would've been acquired
	//by playing through the chapters along the path
	public void ApplyStoryNodeValues()
	{
		List<StoryNode> path = new List<StoryNode>();
		StoryNode current = this;
		while(current != null)
		{
			path.Add(current);
			current = current.parent;
		}
		path.Reverse();
		foreach(var node in path)
			node.ApplyStoryNodeValuesSingle();
	}
}
