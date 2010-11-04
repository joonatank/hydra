print 'In python init script'
print 'Creating Ogre SceneNode'
node = SceneNode.create("ogre")
config.addSceneNode(node)
event = TransformationEvent(node)
# TODO should have different nodes for yaw and pitch so that the Ogre would
# rotate around global axes
event.setRotYKeys( KC.NUMPAD6, KC.NUMPAD4 );
event.setRotZKeys( KC.NUMPAD8 , KC.NUMPAD5 );
print 'Adding ogre transformation event to event stack'
if not config.addEvent(event) :
	print 'Python : could not add ogre event'
node = SceneNode.create("CameraNode")
print 'Creating Camera SceneNode'
config.addSceneNode(node)
event = TransformationEvent(node)
event.setTransXKeys( KC.D, KC.A );
event.setTransYKeys( KC.PGUP, KC.PGDOWN );
event.setTransZKeys( KC.S, KC.W );
# TODO yaw missing, but the Application does not allow yaw for cameras because
# it looks funny in multi wall systems
event.setRotYKeys( KC.RIGHT, KC.LEFT );
if not config.addEvent(event) :
	print 'Python : could not add camera event'
