# About

Stories are considered as essential in serious games. However, in practice, stories and gameplay are often developed by separate teams, leading to a sub-optimal match between these. On top of this, there are few easy-to-use dashboards for real-time visual serious game analytics to assess gamer behavior.

The Storyline Scaffolding Dashboard (SSD) supports real-time serious game analytics by explicitly encoding, visualizing, and linking both the story and gameplay as node trees in real-time. This helps in game development by ensuring that there is a perfect match between the story and gameplay. Moreover, this design allows for visually aesthetic real-time player analytics to view a player through a game in real-time and evaluate their performance.

The SSD is written in modern C++, using C++11 features, in Qt and is based on the ZodiacGraph, a node graph user interface module ([https://github.com/clemenssielaff/ZodiacGraph](https://github.com/clemenssielaff/ZodiacGraph)). Both the SSD and ZodiacGraph are released under the MIT license.

This project was funded from the European Union&#39;s Horizon 2020 research and innovation program under grant agreement No 732599, the _REVEAL_ project ([www.revealvr.eu](http://www.revealvr.eu/)).

# Acknowledgments

Thank you to the participants of the REVEAL project for suggestions for features and testing the tool. Also, thank you to Clemens Sielaff for developing ZodiacGraph and making it freely available for use as a base.

# Executable

The executable file is in the /bin folder (SSD.exe) along with the requirements to run.

# Requirements

Although untested, approximate minimal hardware requirements for running the SSD are:

- 256 MB of RAM
- 500 MHz CPU, 1 GHz
- OpenGL ES 2.0 support

To launch, the SSD needs the &quot;commandsandparams.json&quot; file. This contains the commands, and parameters, for the gameplay graph. An example file is provided. More information is available in the &quot;Commands&quot; subsection of the &quot;Gameplay Graph&quot; section.

# Installation

## Qt Creator

Open the project file &quot;Dashboard.pro&quot; in QtCreator. After configuring the build directories, choose the play option and the dashboard should open.

## QMake

Open the Qt command line to make sure that all Qt-libraries are available on the path, call vcvarsall.bat and define the target machine type (x86 or x64). In the repository, run:

_qmake
 nmake release
 release\Story\_Scaffolding\_Dashboard.exe_

# Videos

The gameplay graph lighting up and sidebar updating as a player progresses through the game (old version) – [https://www.youtube.com/watch?v=4Jw7JMxfvuo](https://www.youtube.com/watch?v=4Jw7JMxfvuo).

# Full Documentation

Full Doxygen documentation to come soon.

# Story Linking Functionality

## Story Graph

### Overview

The SSD&#39;s representation of a story is based on Story Grammar Theory [Thorndyke, 1977, DOI: [https://doi.org/10.1016/0010-0285(77)90005-6](https://doi.org/10.1016/0010-0285(77)90005-6)], which defines a story into four main parts: setting, theme, plot, and resolution. Subsequently, these parts can be rewritten into more sections using 10 available rules, as shown in the figure below.

![](RackMultipart20200612-4-pevdq5_html_a8186eb3ac2601a2.png)

For example, settings consists of characters, location, and time and the plot consists of episodes that contain attempts, outcomes, and a subgoal. For example, in The Chantry, an event is &quot;smallpox is a horrifying disease&quot; and an outcome of one episode is &quot;James developed a mild fever, but quickly recovered&quot;. These rules provide a hierarchical representation of the story, known as the story graph, providing a mental schema of how a story unfolds. The graph will be explained in detail using parts of the story in the Example subsection.

### Usage

#### Creating a Story Graph

To create a story within the tool, first select the &quot;Add Story Graph&quot; option.

![](RackMultipart20200612-4-pevdq5_html_3718c7a10fa5d17a.png)

#### Adding Nodes

This will create the base of a Story Graph (setting, theme, plot, and resolution), more nodes are added by right-clicking on a node to and selecting an option to create a child.

![](RackMultipart20200612-4-pevdq5_html_ece6f8bc961ba509.png)

This can be done for all nodes that accept children as defined in the schema.

![](RackMultipart20200612-4-pevdq5_html_bc5e097995cfd4d5.png) ![](RackMultipart20200612-4-pevdq5_html_6876b30c8e2993cf.png) ![](RackMultipart20200612-4-pevdq5_html_94b09798fba5d58b.png) ![](RackMultipart20200612-4-pevdq5_html_627944e0e03f870a.png)

#### Editing Node Names and Descriptions

When a node is clicked on, it can be edited in the sidebar, as shown below. The name (ID) of the node and description (story sentence) can be edited in the sidebar. This is not possible for the prefix. This also shows the parent and children of the node, as well as links to gameplay nodes (see the linking section). A node name change in the sidebar will be reflected in the graph.

![](RackMultipart20200612-4-pevdq5_html_f24ee35120e08788.png)

#### Removing Nodes

If a node has no children and is not mandatory, it can be removed. This can be done in two ways:

- Selecting a node and clicking the delete node button in the sidebar (as shown above)
- Clicking with the middle mouse button on the node

#### Saving a Story Graph

To save the story graph, select the &quot;Story Graph&quot; menu and select &quot;Save Story&quot;.

![](RackMultipart20200612-4-pevdq5_html_f71bcd32b5c5d7ba.png)

The story will be saved as a JSON file in the location chosen through the dialog box.

![](RackMultipart20200612-4-pevdq5_html_ea1e3397a0fd538a.png)

#### Loading a Story Graph

To load the story graph, select the &quot;Story Graph&quot; menu and select &quot;Load Story&quot;.

![](RackMultipart20200612-4-pevdq5_html_45300a04f2cf8a4a.png)

A dialog box will appear to load the story graph.

![](RackMultipart20200612-4-pevdq5_html_14c39eebc611f7bd.png)

Once selected, this will load the story graph into the tool.

![](RackMultipart20200612-4-pevdq5_html_b11e995af88184b8.png)

#### JSON Schema

**TO BE WRITTEN LATER**

### Example

#### Story

1. The Chantry
2. is a country house
3. in Berkeley,
4. near to the River Severn in the county of Gloucestershire, England.
5. It stands between Berkeley Castle and the church
6. and was originally built as housing for priests
7. In 1823
8. it was the home to a physician and widower
9. called Edward Jenner
10. Jenner also had a second home in Cheltenham
11. Jenner&#39;s main work was on birds and cuckoos
12. Yet he and his colleagues carried out many interesting experiments
13. Which scared a lot of people
14. Including improving grass growth using human blood
15. And launching balloons
16. Jenner turned his attention to the study of smallpox
17. after observing that dairymaids appeared to be protected from smallpox after having suffered from cowpox
18. He was told this by Sarah Nelmes
19. a milkmaid
20. Cowpox resembles a mild form of smallpox
21. Smallpox is a horrifying disease
22. which killed 1 in 5 people who caught it
23. and scarred and/or blinded many other
24. It affected both rich and poor people
25. Queen Mary II of England
26. Was killed
27. and Beethoven and Mozart
28. Were left scarred
29. Some people couldn&#39;t live with the terrible scarring and took their own lives
30. Once someone had had smallpox they couldn&#39;t get it again
31. Smallpox scars were an asset to servants seeking employment as it showed they couldn&#39;t bring the disease into a household
32. Jenner was immune to smallpox
33. as he went through the process of inoculation when he was younger
34. Inoculation involved deliberately infecting someone with smallpox by scratching pus from a smallpox scar into their bloodstream
35. This induced a milder form of smallpox which most people survived
36. and gave them future immunity to the disease
37. Inoculation was used in Africa and Turkey before it came to Europe and America.
38. Lady Mary Wortley Montagu
39. brought the practice of variolation to England from Turkey
40. The Reverend Cotton Mather
41. brought inoculation to The Americas after being taught the practice by an African slave
42. However, other preachers regarded thee process as sinful
43. Jenner took the practice one step further by using an inoculant which came from a related non-fatal disease (cowpox)
44. In 1796
45. Jenner extracted the pus from cowpox lesions on a milkmaid&#39;s arm
46. and scratched this onto the arm of an 8-year old boy called James Phipps
47. to see if this made him immune to smallpox
48. James developed a mild fever but quickly recovered.
49. Eight weeks later Jenner injected James with pus from a fresh smallpox lesion
50. No disease developed,
51. and Jenner concluded that Phipps was now protected against smallpox
52. Jenner was working at the time of the Napoleonic Wars between Britain and France
53. but still helped the French people
54. and was held in high regard by Napoleon
55. who spent thousands of francs promoting vaccination throughout his empire

#### Story Graph

![](RackMultipart20200612-4-pevdq5_html_f2c155f59506ee53.png)

##### Setting

Setting consists of characters, locations, and times. These accept unlimited states (referred to as details in the SSD). For example, The Chantry (location) is described as &quot;a country house&quot; (detail/state).

##### Theme

The theme consists of events, which have happened to set up the story, and goals, the desired state. Events and goals can have children: sub-events and sub-goals, respectively. These children can also have children. For example, &quot;Smallpox is a horrifying disease&quot; is an event with the sub-event &quot;which killed 1 in 5 people who caught it&quot; and &quot;Once someone had had smallpox they couldn&#39;t get it again&quot; is a goal with the sub-goal &quot;Smallpox scars were an asset to servants seeking employment as it showed they couldn&#39;t bring the disease into a household&quot;.

##### Plot

The plot is the most complicated part of the story graph and covers the key events in the story.

Attempts and outcomes can also have sub-episodes. For example, an episode that goes through other types of immunity before vaccination has a sub-episode discussing inoculation.

##### Resolution

The resolution is the end of the story and consists of events and states. The events reference the events that happened at the end of the story, such as &quot;the British Parliament eventually granted Edward Jenner a grant of £10,000 for his smallpox research&quot; and states reference the end state, such as &quot;The smallpox virus was finally eradicated in 1980 following a global immunization campaign led by the World Health Organisation&quot;.

#### JSON Snippet

&quot;episodes&quot;: [

{

&quot;attempts&quot;: [

{

&quot;description&quot;: &quot;Jenner was working at the time of the Napoleonic Wars between Britain and France&quot;,

&quot;id&quot;: &quot;ATT\_WARS&quot;

}

],

&quot;description&quot;: &quot;Napoleonic Wars&quot;,

&quot;id&quot;: &quot;EP\_NAPOLEON&quot;,

&quot;outcomes&quot;: [

{

&quot;description&quot;: &quot;who spent thousands of francs promoting vaccination throughout his empire&quot;,

&quot;id&quot;: &quot;OUT\_EMPIRE&quot;

},

{

&quot;description&quot;: &quot;and was held in high regard by Napoleon&quot;,

&quot;id&quot;: &quot;OUT\_HIGHREGARD&quot;

}

],

&quot;subGoal&quot;: {

&quot;description&quot;: &quot;but still helped the French people&quot;,

&quot;id&quot;: &quot;SUBG\_HELPED&quot;

}

}

## Gameplay Graph

### Overview

The Gameplay Graph is a behavior tree that triggers sequences of voiceovers, sound effects, animations and changes in lighting as well as the functionality to unlock new areas of the environment. When a gameplay node is accessed, through interactions with the game environment, it triggers optional command blocks. Each command block dispatches a list of commands.

Gameplay nodes can be either:

- Locked (although unlockable if all dependencies met)
- Being Unlocked
- Already Unlocked

A gameplay node may depend upon previous nodes whose state determines if this node is available. Therefore, this graph represents the progress of the player through the game. Different states trigger different events so the event triggered for a locked node can be different to the event for an unlocked node.

Decorators are used to make dependencies more complicated; these are:

- Sequencer (logical AND operation)
- Selector (logical OR operation)
- Inverter (logical NOT operation)
- Leaf (a gameplay node test to check if it is active)

As mentioned, these gameplay nodes trigger events, which are represented by commands. These are represented by the ID of the command/event to be triggered and a list of parameters.

Multiple game graphs can be created, saved and loaded within the tool. This is so multiple files can be created for different areas of the game should developers wish to do so.

### Usage

#### Creating a Gameplay Node

To create a gameplay node within the tool, first select the &quot;Add Gameplay Node&quot; option.

![](RackMultipart20200612-4-pevdq5_html_d76063ff6919a4a0.png)

#### Editing a Gameplay Node

When a node is clicked on, it can be edited in the sidebar, as shown below. The name (ID) of the node and (optional) description can be edited in the sidebar and the three possible command paths (node is locked, being unlocked or already unlocked) can be added to or changed. The filename that the node will be saved to, along with other gameplay nodes with the same filename, can also be set. The links to story nodes and dependencies (see Linking section) are also visible.

![](RackMultipart20200612-4-pevdq5_html_cb751a6ab76c797a.png)

For each of the three options for triggering commands:

- OnUnlock – Node being unlocked
- OnFail – Node locked
- OnUnlocked – Node already unlocked

lists of commands can be added (using the plus icon), removed (using the minus icon) or edited. The commands are selectable through dropdown boxes, which, in turn, show the parameters that can be set through simple text boxes.

These commands and parameters are set in the &quot;commandsandparams.json&quot; file (see Commands and Parameters File subsection).

#### Loading Gameplay Graphs

To load gameplay graphs, select the &quot;Gameplay Graph&quot; menu and select &quot;Load Gameplay Graph&quot;.

![](RackMultipart20200612-4-pevdq5_html_18391b7b7d7f15b5.png)

A dialog box will appear to load the gameplay graphs.

![](RackMultipart20200612-4-pevdq5_html_25c0ec36610daf99.png)

When loading, the user will need to place these graphs in the desired order. ![](RackMultipart20200612-4-pevdq5_html_cb1fdb3d320396ef.png)

The graphs will then show in the tool.

![](RackMultipart20200612-4-pevdq5_html_221a2ae71a4cce99.png)

#### Saving Gameplay Graphs

To save gameplay graphs, select the &quot;Gameplay Graph&quot; menu and select &quot;Load Gameplay Graph&quot;.

![](RackMultipart20200612-4-pevdq5_html_48cb0455f69bd649.png)

The gameplay graphs will be saved as JSON files in the location chosen through the dialog box.

![](RackMultipart20200612-4-pevdq5_html_2d68d05c7569ba2b.png)

#### Commands and Parameters File

As mentioned at the beginning of this guide, the &quot;commandsandparams.json&quot; file is needed to start the tool and holds the information for gameplay commands and associated parameters. A sample file is provided. An example is provided.

This file contains an array of possible parameters, such as the &quot;Time&quot; parameter (how many seconds to wait before triggering a command):

{
 &quot;label&quot;: &quot;Time&quot;,
 &quot;id\_name&quot;: &quot;time&quot;,
 &quot;type&quot;: &quot;float&quot;
 }

As shown above, this object consists of a label that is shown in the tool, the id for the JSON file and the type of value that is accepted.

A separate array contains the commands, such as the &quot;Add Subtitle&quot; command:

{
 &quot;label&quot;: &quot;Add Subtitle&quot;,
 &quot;id\_name&quot;: &quot;add\_subtitle&quot;,
 &quot;params&quot;: [&quot;time&quot;, &quot;text\_id&quot;]
 }

As shown above, this object consists of a label that is shown in the tool, id for the JSON file and the parameters that it uses.

#### JSON Schema

**TO BE WRITTEN LATER**

### Example

#### Description

The &quot;Chantry&quot; gameplay graph is a gameplay graph for a task in &quot;The Chantry&quot; game, which starts when a dining room door is accessed. When this first gameplay node (Act2\_JHH\_DiningDoor) is triggered (on\_unlock), the player is played an audio narrative and a task list appears, showing an item to be found and turned over (Act2\_JB\_CastlePicture). If the player tries the door again (on\_unlocked), they hear the sound of a locked door trying to be opened. If the player finds the item (Act2\_JHH\_CastlePictureEarly) before the first node has been unlocked (on\_fail), they are played an audio file telling them the item isn&#39;t useful right now. If the first node is unlocked (on\_unlock), an audio narrative plays telling the player that the item is useful. On turning the item over (Act2\_JHH\_CastlePicture), another audio narrative plays and the door unlocks. Putting the item down completes the task (Act2\_JHH\_Chantry\_Complete). Picking up and turning the item over later (on\_unlocked), plays the audio narrative again.

#### Graph

![](RackMultipart20200612-4-pevdq5_html_2c50f0a3b1d98ba3.png)

#### JSON Snippet

[

{

&quot;id&quot;: &quot;Act2\_JHH\_DiningDoor&quot;,

&quot;on\_unlock&quot;: [

{

&quot;cmd&quot;: &quot;focus\_screen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;add\_subtitle&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;play\_sound&quot;,

&quot;id&quot;: &quot;narr/Act2\_JHH\_DiningDoor&quot;,

&quot;time&quot;: 0

}

],

&quot;on\_unlocked&quot;: [

{

&quot;cmd&quot;: &quot;play\_commands&quot;,

&quot;id&quot;: &quot;door\_barred\_gen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;show\_cl\_progress&quot;,

&quot;id&quot;: &quot;LS\_CL\_CHANTRY&quot;,

&quot;time&quot;: 0

}

],

&quot;story\_tags&quot;: [

&quot;LOC\_CHANTRY&quot;

]

},

{

&quot;id&quot;: &quot;Act2\_JHH\_CastlePictureEarly&quot;,

&quot;on\_fail&quot;: [

{

&quot;cmd&quot;: &quot;add\_subtitle&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;focus\_screen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;play\_sound&quot;,

&quot;id&quot;: &quot;narr/Act2\_JHH\_CastlePictureEarly&quot;,

&quot;time&quot;: 0

}

],

&quot;on\_unlock&quot;: [

{

&quot;cmd&quot;: &quot;play\_commands&quot;,

&quot;id&quot;: &quot;closer\_look\_gen&quot;,

&quot;time&quot;: 0

}

],

&quot;requirements&quot;: {

&quot;id&quot;: &quot;Act2\_JHH\_DiningDoor&quot;,

&quot;type&quot;: &quot;LEAF&quot;

}

},

{

&quot;id&quot;: &quot;Act2\_JB\_CastlePicture&quot;,

&quot;on\_unlock&quot;: [

{

&quot;cmd&quot;: &quot;focus\_screen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;play\_sound&quot;,

&quot;id&quot;: &quot;narr/Act2\_JB\_CastlePicture&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;add\_subtitle&quot;,

&quot;time&quot;: 0

}

],

&quot;on\_unlocked&quot;: [

{

&quot;cmd&quot;: &quot;play\_commands&quot;,

&quot;id&quot;: &quot;@Act2\_JB\_CastlePicture\_on\_unlock&quot;,

&quot;time&quot;: 0

}

],

&quot;requirements&quot;: {

&quot;id&quot;: &quot;Act2\_JHH\_DiningDoor&quot;,

&quot;type&quot;: &quot;LEAF&quot;

},

&quot;story\_tags&quot;: [

&quot;DET\_CASTLE&quot;

]

},

{

&quot;id&quot;: &quot;Act2\_JHH\_ChantryComplete&quot;,

&quot;on\_unlock&quot;: [

{

&quot;cmd&quot;: &quot;focus\_screen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;play\_commands&quot;,

&quot;id&quot;: &quot;door\_creak\_gen&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;add\_subtitle&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;try\_close\_logic\_node&quot;,

&quot;duration&quot;: 0,

&quot;id&quot;: &quot;DiningRoom\_Door\_Interaction\_Logic&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;try\_open\_logic\_node&quot;,

&quot;duration&quot;: 1.5,

&quot;id&quot;: &quot;DiningRoom\_Door\_Logic&quot;,

&quot;time&quot;: 0

},

{

&quot;cmd&quot;: &quot;play\_sound&quot;,

&quot;id&quot;: &quot;narr/Act2\_JHH\_ChantryComplete&quot;,

&quot;time&quot;: 0

}

],

&quot;requirements&quot;: {

&quot;id&quot;: &quot;Act2\_JB\_CastlePicture&quot;,

&quot;type&quot;: &quot;LEAF&quot;

},

&quot;story\_tags&quot;: [

&quot;DET\_PRIESTS&quot;

]

}

]

## Linking

To meet the key aim of the tool: being able to structure and visualize the story (educational content) within the game, the gameplay graph must be linked to the story graph to show where each of the story elements are present in the game. Moreover, to structure the gameplay graph, the same functionality must be available to add dependencies to different gameplay nodes. Therefore, nodes will be linked as either gameplay-story links or, when both nodes are gameplay, requirements lists, a list of nodes that must be unlocked before the selected node is unlockable).

To link a gameplay node to another node, story or gameplay, the gameplay node which needs to be linked to a story node or needs a requirements list to be created must be right-clicked on.

![](RackMultipart20200612-4-pevdq5_html_47e0800a2d5a3302.png)

Clicking the option to link this node to other nodes, shows a dialog window. This window contains separate tabs that list gameplay and story nodes respectively.

![](RackMultipart20200612-4-pevdq5_html_f654db904d7d0904.png) ![](RackMultipart20200612-4-pevdq5_html_835baec3174947b4.png)

Checking the boxes for story nodes will result in simple links being made to story nodes and checking the three-state boxes for other gameplay nodes will result in requirements links being made between these nodes; this includes adding a sequence decorator if more than one node has been checked to show that this node requires a sequence of nodes to be unlocked or an inverter if the third (negative) state of the checkbox is selected.

### Linking Analytics

To provide feedback to the user, story nodes that are not referenced in the gameplay graph will have red-colored labels, rather than green labels, to show that this part of the story is missing from the game.

![](RackMultipart20200612-4-pevdq5_html_b5bb7b2b01323c2.png)

On top of this, the sidebar shows calculations of story completeness:

- Number of Story Nodes with Connections
- Number of Story Nodes Story Connections
- Average Number of Gameplay Connections
- Number of Gameplay Nodes with Connections
- Number of Gameplay Nodes without Connections
- Average Number of Story Connections
- Total Number of Gameplay-Story Connections

![](RackMultipart20200612-4-pevdq5_html_7805504dd8d70069.png)

These two aspects make it makes visible and computable how complete –and where – the story is represented in the gameplay layer.

## Example of Use

A gameplay node, &quot;Act2\_JHH\_ CastlePicture&quot;, has the following audio narrative attached: &quot;_It was called &quot;The Chantry&quot;, from having, in former times, been in the possession of certain monks. It is contiguous to the churchyard of Berkeley Castle, and the tower of St Mary&#39;s Church over-hangs the southern boundary of the shrubbery_&quot;. This is related to a node in the story graph, &quot;DET\_CASTLE&quot; (see story graph example point 5), which reads: &quot;_It stands between Berkeley Castle and the church_&quot;. Therefore, this gameplay node is linked to the story node.

# Real-time Analytics

As a result of the gameplay-story linking and the presentation of these two nodes, real-time analytics for following and assessing a player, as they move through a game, become available.

## In-Game Tasks

### Overview

In most games, players must complete tasks to proceed. To make use of the live analytics, using the node trees, these tasks are imported into the tool. Tasks generally consist of the following:

1. Task Name
2. Start Point
3. End Point
4. Objective(s) with name

The tool accepts a JSON file that contains these elements:

1. Task Name = text\_id string
2. Start Point = begin\_dep string
3. End Point = complete \_dep string
4. Objective(s) with name = narrative\_deps array with narr\_id string

#### Usage

Go to the analytics menu and select &quot;Add/Edit Tasks&quot;.

![](RackMultipart20200612-4-pevdq5_html_252d98438a7f5f21.png)

A dialog box will appear giving options to load and save a task, along with options for spatial graph and a starting node (see spatial graph subsection in the lostness section).

![](RackMultipart20200612-4-pevdq5_html_de51198e78e2abcd.png)

Tasks are loaded through a file dialog.

![](RackMultipart20200612-4-pevdq5_html_6ab11d3750a47ee3.png)

And result in the necessary variables for tasks being loaded into the tool (calculate lostness and the minimum step variables are explained in the lostness section).

![](RackMultipart20200612-4-pevdq5_html_9dbea49d59c1b8dc.png)

These tasks can then be used for real-time analytics.

#### Example

(Shortened to remove variables used in the game)

{

&quot;narrative\_deps&quot;: [

{

&quot;narr\_id&quot;: &quot;Act2\_JHH\_CountyMap&quot;,

},

{

&quot;narr\_id&quot;: &quot;Act2\_EJ\_LastLetter&quot;,

&quot;subtitle&quot;: &quot;LS\_CL\_GLOUCESTERSHIRE\_LASTLETTER&quot;,

&quot;decal\_id&quot;: &quot;Act2\_EJ\_LastLetter\_Decal&quot;

}

],

&quot;text\_id&quot;: &quot;LS\_CL\_GLOUCESTERSHIRE&quot;,

&quot;begin\_dep&quot;: &quot;Act2\_JHH\_BreakfastDoor&quot;,

&quot;complete\_dep&quot;: &quot;Act2\_JHH\_HomeComplete&quot;

}

##### JSON Schema

To Follow

### Lostness

To assess how well a player has completed tasks, the lostness measure is used within the tool. This measure is originally from hypertext and used to identify disorientation. It has shown to be successful in predicting success in information-seeking tasks in hypertext. It is possible to implement in games if tasks/objectives can be defined in a minimum number of steps and steps a player has taken can be logged. The values needed are:

- The minimum number of steps needed to complete a task/objective (R)
- The total number of steps taken (S)
- The unique number of steps taken (N)

For a given task, the lostness (L) is defined as follows:

**,**

This returns a value between 0 and √2, which is converted to a percentage in the SSD. A player with a 0% lostness value has taken the perfect path and a value close to 100% indicates that a player is completely lost and highly disoriented.

#### Global Lostness

Manually defined, tasks

#### Local Lostness

Pathfinding, objectives

#### Calculation Options

An option exists, in the tasks dialog, to calculate lostness in the tool or from the game. This lostness value can be received in an interaction object (see Interaction Object subsection in the Network section) or within the tool.

For global lostness, the tool uses the minimum steps value in the tasks dialog. Local lostness makes use of a pathfinding algorithm to calculate this value (see next section). Both measures use a list of accepted verbs received through an interaction object to calculate the S and N value for the player:

- Moving to a new location – &quot;moved to&quot;
- Picking up an item – &quot;picked up&quot;
- Examining (turning over) an item – &quot;examined&quot;

##### Spatial Graphs and Pathfinding

How it works etc.

## Network

To receive the necessary information for live analytics, information is transferred to the tool from the game.

### Connection

The SSD must connect to a TCP server, as a client, that will send JSON interaction objects. This server can either run within the game or the game can send these objects to a server, which will send these to the SSD.

To connect to a server, click &quot;Enable Analytics Mode&quot;, then, from the &quot;Analytics&quot; menu, select &quot;Connect to server&quot;.

![](RackMultipart20200612-4-pevdq5_html_f61060f9a05b7350.png)

This dialog will appear for an IP and port number (domains are also accepted).

![](RackMultipart20200612-4-pevdq5_html_9653a9ed963a4cf9.png)

A dialog will appear showing if the connection is successful. Connection issues are not detailed here.

### Interaction Object

The SSD uses the Serious Games Interactions Model (SGIM). This consists of:

- Player name
- Action (verb)
- Object
- Timestamp (ISO 8601)
- Optional value (result)

Common verbs used in the SSD are

- &quot;moved to&quot; – When a player moved
- &quot;picked up&quot; – When a player picks up an object
- &quot;examined&quot; – When a player examines (turns over) an object
- &quot;started&quot; – When a player starts a task
- &quot;completed&quot; – When a player starts a task
- &quot;attempted&quot; – When a player attempts to unlock a narrative node (contains result of fail, unlock or unlocked), which can indicate an objective has been found

For more information, and the correct format, see [https://github.com/e-ucm/xapi-seriousgames](https://github.com/e-ucm/xapi-seriousgames).

Additionally, the SSD allows an optional &quot;lostness&quot; variable for an &quot;unlock&quot; or &quot;completed&quot; verb when the game calculates lostness rather than the tool.

#### Example Interaction

{

&quot;actor&quot;: &quot;Player 1&quot;,

&quot;object&quot;: &quot;Act1\_EJ\_BerkleyHome&quot;,

&quot;result&quot;: {

&quot;result&quot;: &quot;unlock&quot;

},

&quot;timestamp&quot;: &quot;2019-10-26T14:57:44Z&quot;,

&quot;verb&quot;: &quot;attempted&quot;

}

## Logging

When the JSON data is received from the server, this is logged in full to a text file to be examined in full at a later time if necessary. This will contain all events in case different types of analytics or in-depth examination of the gameplay data if necessary. In addition to this, a log widget is available at the bottom of the tool which shows human-readable sentences derived from the JSON data. Through this, a teacher or researcher can see the current progress of a player in the game in text format.

![](RackMultipart20200612-4-pevdq5_html_3d6aa21d1f69a922.png)

## &quot;Lighting-Up&quot;

In addition to the primitive text logger, the JSON data is also used to &quot;light up&quot; the gameplay and story graphs. This will be done when a gameplay node is unlocked (interaction object with &quot;attempted&quot; verb and &quot;unlock result&quot;) which then allows other nodes to be unlocked and will use the following color scheme:

- Red – Node locked
- Blue – Node can be unlocked
- Green – Node unlocked

![](RackMultipart20200612-4-pevdq5_html_20b6aa8acb15184f.png)

In addition, story nodes attached to these nodes will also be highlighted when these have been accessed. These will simply be red when they have not been accessed and green when they have. This shows the gameplay events that the player has interacted with and the story information/educational content they have been exposed to and, hopefully, learned.

![](RackMultipart20200612-4-pevdq5_html_32a84209064943b0.png)

## Sidebar

When the user enters analytics mode. Tasks populate in the sidebar. Progress bars are used to show the progress the player has made through the task as well as the local lostness for the starting objective, other objectives and the full task (local or global as defined by the user). Tasks that have been completed have green text, in-progress have yellow text, and tasks that have not been started have red text. Similarly, objectives that have been found are green and red if not found. This gives a full view of player progress and performance through the game.

![](RackMultipart20200612-4-pevdq5_html_5b5e6cbd3791fa77.png)

# Additional Notes

## Undo

An undo function was taken into account from the start of the project, due to it being a key feature of the new tool. The Undo Framework, which Qt includes for easy implementation of undo and redo functionality, is powerful and straightforward, however, developing an undo/redo system towards the end of the project, where all of the main functionality is included before the undo functionality, is extremely difficult. Based on this knowledge, whenever a new feature was added, a new redo/undo class was created and tested. All major editing functions, including creating and deleting nodes and editing node properties, can easily be done and undone using keyboard shortcuts or the menu options.

## Loading Log Files

So that log files can be visualized at a later time, there is an option to load a log file in the Analytics menu. This will show the data the same as if it had just been sent by a game.

## Linking Display

There are different ways of displaying links in the SSD. Links can be removed between story nodes, gameplay nodes, and story and gameplay nodes. In analytics mode, links between story and gameplay nodes are faded, only shown explicitly in the location where the player currently is.

# Future Features

Although the tool is functional, in addition to bug fixes and polish, the following features are desirable:

- The undo functionality is only mainly used when carrying out actions in the sidebar. This will be extended to the full application
- The ability to send commands from the SSD to the game for adapting a game to the players&#39; skill level depending on user input or lostness values
- More intuitive linking, based on feedback from a user study – Probably pressing a key combination and clicking on different nodes
- The ability to search for story text when linking a gameplay node to a story node
- Spacing out both node graphs properly. This does not currently work and has some problems
- Error checking for specific types of input
- Different input types for command parameters, especially numbers and colors
- The ability to specify commands and parameters within the SSD and save them
- The ability to create and edit tasks (names etc.) within the SSD
- Functionality to specify accepted lostness verbs in the tasks window
- Replacing lostness with efficiency, inverting the measure – User feedback found lostness confusing alongside progress
- Exporting values (progress, lostness, etc.) to CSV for further analysis
- Functionality to re-play a game session in real-time/sped up rather than a static result

# Disclaimer

As the REVEAL Project was completed at the end of 2018, improvements are sporadic and may not be completed. On top of this, because of the fluid nature of the project, code was updated quickly and some of the code was taken from the initial version and reused. Therefore, the application is not fully optimized and may suffer from bugs. The code is being cleaned but, again, this is a gradual progress, mainly as a personal project, and may not be completed.
