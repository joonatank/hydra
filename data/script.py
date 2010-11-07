def addOgreEvent() :
	print 'Creating Ogre SceneNode'
	node = SceneNode.create("ogre")
	config.addSceneNode(node)
	event = TransformationEvent(node)
	# TODO should have different nodes for yaw and pitch so that the Ogre would
	# rotate around global axes
	event.setRotYKeys( KC.NUMPAD6, KC.NUMPAD4 )
	event.setRotZKeys( KC.NUMPAD8 , KC.NUMPAD5 )
	print 'Adding ogre transformation event to event stack'
	if not config.addEvent(event) :
		print 'Python : could not add ogre event'

def addCameraEvent() :
	print 'Creating Camera SceneNode'
	node = SceneNode.create("CameraNode")
	config.addSceneNode(node)
	event = TransformationEvent(node)
	event.setTransXKeys( KC.D, KC.A )
	event.setTransYKeys( KC.PGUP, KC.PGDOWN )
	event.setTransZKeys( KC.S, KC.W )
	# TODO yaw missing, but the Application does not allow yaw for cameras because
	# it looks funny in multi wall systems
	event.setRotYKeys( KC.RIGHT, KC.LEFT )
	if not config.addEvent(event) :
		print 'Python : could not add camera event'

def addQuitEvent() :
	print 'Creating Quit Event'
	action = event_manager.createOperation( 'QuitOperation' )
	print 'Python Action type = ' + action.type
	action.config = config 
	#trigger = event_manager.createTrigger( 'KeyTrigger' )
	trigger = event_manager.createKeyTrigger( )
	# FIXME this segfaults
	print 'Python Trigger type = ' + trigger.type
	#print trigger.type
	#print getKeyName( KC.W )
	trigger.key = KC.Q
	# Example of using released key instead of pressed (which is default)
	trigger.released = True

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
	#trigger = event_manager.createKeyTrigger( )
	# FIXME this segfaults
	print 'Python Trigger type = ' + trigger.type
	trigger.key = KC.R
	# Example of using released key instead of pressed (which is default)
	trigger.released = True

	# Create the Event
	event = event_manager.createEvent( 'Event' )
	event.operation = action
	event.addTrigger( trigger )
	if not event_manager.addEvent( event ) :
		print 'Python : Event could not be added to EventManager'

# TODO create PrintOperation in python and register it into Event Manager

print 'In python init script'
#addOgreEvent()
#addCameraEvent()
addQuitEvent()
addReloadEvent()
