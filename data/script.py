# TODO these should be fixed, the interface has changed considerably
# FIXME the EventManager does not support these yet
def addRotationEvent(node) :
	# TODO should print the node name
	print 'Creating Rotation event on node = '
	event = event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# TODO should have different nodes for yaw and pitch so that the Ogre would
	# rotate around global axes
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.NUMPAD6
	trigger_neg.key = KC.NUMPAD4
	event.setRotYtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.NUMPAD8
	trigger_neg.key = KC.NUMPAD5
	event.setRotZtrigger( trigger_pos, trigger_neg )
	print 'Adding ogre transformation event to event stack'
	if not event_manager.addEvent(event) :
		print 'Python : could not add ogre event'

# TODO these should be fixed, the interface has changed considerably
# FIXME the EventManager does not support these yet
def addTranslationEvent(node) :
	# TODO should print the node name
	print 'Creating Translation event on node = '
	event = event_manager.createEvent( 'TransformationEvent' )
	event.scene_node = node
	# TODO this is bit verbose and repeated often move it to python function
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.D
	trigger_neg.key = KC.A
	event.setTransXtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.PGUP
	trigger_neg.key = KC.PGDOWN
	event.setTransYtrigger( trigger_pos, trigger_neg )
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.S
	trigger_neg.key = KC.W
	event.setTransZtrigger( trigger_pos, trigger_neg )

	# TODO yaw missing, but the Application does not allow yaw for cameras because
	# it looks funny in multi wall systems
	trigger_pos = event_manager.createTrigger( 'KeyTrigger' )
	trigger_neg = event_manager.createTrigger( 'KeyTrigger' )
	trigger_pos.key = KC.RIGHT
	trigger_neg.key = KC.LEFT
	event.setRotYtrigger( trigger_pos, trigger_neg )

#	event.setTransXKeys( KC.D, KC.A )
#	event.setTransYKeys( KC.PGUP, KC.PGDOWN )
#	event.setTransZKeys( KC.S, KC.W )
#	event.setRotYKeys( KC.RIGHT, KC.LEFT )
	if not event_manager.addEvent(event) :
		print 'Python : could not add event'

def addHideEvent(node) :
	print 'Creating Hide Event'
	action = event_manager.createOperation( 'HideOperation' )
	action.scene_node = node 
	trigger = event_manager.createTrigger( 'KeyTrigger' )
	trigger.key = KC.H

	event = event_manager.createEvent( 'Event' )
	event.operation = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'


def addQuitEvent() :
	print 'Creating Quit Event'
	action = event_manager.createOperation( 'QuitOperation' )
	print 'Python Action type = ' + action.type
	action.config = config 
	# Example of using pressed or released key instead of both (the default)
	# you need to create either KeyReleasedTrigger or KeyPressedTrigger
	trigger = event_manager.createTrigger( 'KeyReleasedTrigger' )
	print 'Python Trigger type = ' + trigger.type
	trigger.key = KC.Q
	# KeyTrigger will respond to both

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

def addReloadEvent() :
	print 'Creating Reload Scene Event'
	action = event_manager.createOperation( 'ReloadScene' )
	print 'Python Action type = ' + action.type
	action.config = config 
	trigger = event_manager.createTrigger( 'KeyTrigger' )
	# FIXME this segfaults
	print 'Python Trigger type = ' + trigger.type
	trigger.key = KC.R
	# Example of using released key instead of pressed (which is default)
	trigger.released = True

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	# Setting a time limit how often this Event can happen
	event.time_limit = 5
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

# TODO create PrintOperation in python and register it into Event Manager

print 'In python init script'
print 'Adding config events'
addQuitEvent()
addReloadEvent()

print 'Creating Ogre SceneNode'
ogre = SceneNode.create("ogre")
config.addSceneNode(ogre)
addHideEvent(ogre)
addRotationEvent(ogre)

print 'Creating Camera SceneNode'
camera = SceneNode.create("CameraNode")
config.addSceneNode(camera)
addTranslationEvent(camera)

