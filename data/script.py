# TODO this should create SceneNode object (using factory)
# then it should create TransformationEvent
# attach scene node to it
print 'In python init script'
print 'Creating Ogre SceneNode'
# TODO need a factory method here
node = SceneNode.create("ogre")
config.addSceneNode(node)
event = TransformationEvent(node)
event.setRotYKeys( KC.NUMPAD6, KC.NUMPAD4 );
event.setRotZKeys( KC.NUMPAD8 , KC.NUMPAD5 );
print 'Adding ogre transformation event to event stack'
if not config.addEvent(event) :
	print 'Python : could not add ogre event'
