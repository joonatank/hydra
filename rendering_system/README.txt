A copy of the Ogres GL rendering system.
- We will stip it of functionality we don't need.
- Add stereo and easy to use fbo support
- Create both OpenGL 2 and OpenGL3 versions of it.

There are multiple reasons for writing our own or making a copy of the Ogres
rendering system.
- Easier to implement certain features we want like one huge screen and 
rendering all viewpoints using virtual windows (fbos)
- Trying to understand OpenGL more and the bottlenecks in our use case
- Removing useless legacy functionality. Like support for anything older than
OpenGL 2.0 and GPUs without FBOs.
- Removing dependencies to Ogre, as with a custom rendering engine
we only need to replace Ogre's functionality as a SceneGraph.
