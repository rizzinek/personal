using UnityEngine;
using System.Collections.Generic;
using System.Linq;

//objects on the scene that the player can interact with
//while in free-roaming mode
//The interaction is restricted by the player's selected tool and by different conditions
//interactive objects change their visual appearance depending on:
//- whether the player has the correct tool selected
//- whether the conditions to apply the tool to the object are met
//- maybe whether the player is looking at the object
//
//when the object is activated, a "game sequence" is started by a TransitiveItemBase
[RequireComponent(typeof(Collider))]
public class InteractiveObjectHuman : MonoBehaviour
{
	public List<IOHumanTransition> transitions = new List<IOHumanTransition>();
	private List<IOHumanTransition> _currentToolTransitions = new List<IOHumanTransition>();

	void Start()
	{
		InteractionHandler.Instance.handlerHuman.RegisterInteractiveObject(this);
		SelectedToolStateChanged(null, false);
	}

	public void SelectedToolStateChanged(Tool selectedTool, bool isUnsheathed)
	{
		_currentToolTransitions = transitions
			.Where(transition => {
				if(selectedTool == null || !isUnsheathed)
					return transition.requiredToolId.Length == 0;
				else return transition.requiredToolId == selectedTool.itemNameID;
			})
			.ToList();
	}

	public bool TryToActivate()
	{
		int idx = _currentToolTransitions.FindIndex(item => ConditionCheckBase.IsConditionMet(item.condition));
		if(idx >= 0)
		{
			_currentToolTransitions[idx].itemToStart.StartItem();
			return true;
		}
		return false;
	}

	public bool CanBeActivated()
	{
		return _currentToolTransitions.Count > 0
			&& _currentToolTransitions.FindIndex(item => ConditionCheckBase.IsConditionMet(item.condition)) >= 0;
	}
}
