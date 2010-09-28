#include "config.hpp"

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent )
{}

eqOgre::Config::~Config()
{}

void 
eqOgre::Config::mapData( uint32_t const initDataID )
{
	if( _init_data.getID() == EQ_ID_INVALID )
	{
		EQCHECK( mapObject( &_init_data, initDataID ));
        unmapObject( &_init_data ); // data was retrieved, unmap immediately
	}
    else  // appNode, _initData is registered already
    {
        EQASSERT( _init_data.getID() == initDataID )
	}
}

void
eqOgre::Config::unmapData( void )
{
	// Nothing to do as init data is only mapped once and then unmapped
}

bool
eqOgre::Config::init( uint32_t const )
{
	// Register data
	registerObject( &_init_data );
    
	if( !eq::Config::init( _init_data.getID() ) )
    { return false; }

    return true;
}

/*
bool
eqOgre::Config::exit (void)
{
    const bool ret = eq::Config::exit();

    return ret;
}
*/

/*
bool
eqOgre::Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
		{
            if( _handleKeyEvent( event->data.keyPress ))
            { _redraw = true; }
		}
        break;

        case eq::Event::POINTER_BUTTON_PRESS:
			break;

        case eq::Event::POINTER_BUTTON_RELEASE:
        {
            const eq::PointerEvent& releaseEvent = 
                event->data.pointerButtonRelease;
            if( releaseEvent.buttons == eq::PTR_BUTTON_NONE)
            {
                if( releaseEvent.button == eq::PTR_BUTTON1 )
                {
                    _spinX = releaseEvent.dy;
                    _spinY = releaseEvent.dx;
                    _redraw = true;
                    return true;
                }
                if( releaseEvent.button == eq::PTR_BUTTON2 )
                {
                    _advance = -releaseEvent.dy;
                    _redraw = true;
                    return true;
                }
            }
        }
		break;

        case eq::Event::POINTER_MOTION:
		{
            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                if( _frameData.usePilotMode())
                    _frameData.spinCamera(-0.005f*event->data.pointerMotion.dy,
                                          -0.005f*event->data.pointerMotion.dx);
                else
                    _frameData.spinModel( -0.005f*event->data.pointerMotion.dy,
                                          -0.005f*event->data.pointerMotion.dx);

                _redraw = true;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON2 )
            {
                _advance = -event->data.pointerMotion.dy;
                _frameData.moveCamera( 0.f, 0.f, .005f * _advance );
                _redraw = true;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData.moveCamera(  .0005f * event->data.pointerMotion.dx,
                                       -.0005f * event->data.pointerMotion.dy,
                                       0.f );
                _redraw = true;
            }
		}
		break;

        case eq::Event::WINDOW_EXPOSE:
        case eq::Event::WINDOW_RESIZE:
        case eq::Event::WINDOW_CLOSE:
        case eq::Event::VIEW_RESIZE:
			break;

        default:
            break;
    }

	return true;
}
*/

/*	TODO key events are not supported at the moment
bool
eqOgre::Config::_handleKeyEvent( const eq::KeyEvent& event )
{
    switch( event.key )
    {
        case 'p':
        case 'P':
            //_frameData.togglePilotMode();
            return true;
        case ' ':
            _spinX   = 0;
            _spinY   = 0;
            _advance = 0;
            _frameData.reset();
            _setHeadMatrix( eq::Matrix4f::IDENTITY );
            return true;

        case 'i':
        case 'I':
            _frameData.setCameraPosition( 0.f, 0.f, 0.f );
            _spinX   = 0;
            _spinY   = 0;
            _advance = 0;
            return true;

        case 'o':
        case 'O':
            _frameData.toggleOrtho();
            return true;

        case 's':
        case 'S':
            _frameData.toggleStatistics();
            return true;
            
        case 'f':
        case 'F':
            _freeze = !_freeze;
            freezeLoadBalancing( _freeze );
            return true;

        case eq::KC_F1:
        case 'h':
        case 'H':
            _frameData.toggleHelp();
            return true;

        case 'd':
        case 'D':
            _frameData.toggleColorMode();
            return true;

        case 'c':
        case 'C':
        {
            const eq::CanvasVector& canvases = getCanvases();
            if( canvases.empty( ))
                return true;

            _frameData.setCurrentViewID( EQ_ID_INVALID );

            if( !_currentCanvas )
            {
                _currentCanvas = canvases.front();
                return true;
            }

            eq::CanvasVector::const_iterator i = std::find( canvases.begin(),
                                                            canvases.end(), 
                                                            _currentCanvas );
            EQASSERT( i != canvases.end( ));

            ++i;
            if( i == canvases.end( ))
                _currentCanvas = canvases.front();
            else
                _currentCanvas = *i;
            return true;
        }

        case 'v':
        case 'V':
        {
            const eq::CanvasVector& canvases = getCanvases();
            if( !_currentCanvas && !canvases.empty( ))
                _currentCanvas = canvases.front();

            if( !_currentCanvas )
                return true;

            const eq::Layout* layout = _currentCanvas->getActiveLayout();
            if( !layout )
                return true;

            const eq::View* current = findView( _frameData.getCurrentViewID( ));
            const eq::ViewVector& views = layout->getViews();
            EQASSERT( !views.empty( ))

            if( !current )
            {
                _frameData.setCurrentViewID( views.front()->getID( ));
                return true;
            }

            eq::ViewVector::const_iterator i = std::find( views.begin(),
                                                          views.end(), current);
            EQASSERT( i != views.end( ));

            ++i;
            if( i == views.end( ))
                _frameData.setCurrentViewID( EQ_ID_INVALID );
            else
                _frameData.setCurrentViewID( (*i)->getID( ));
            return true;
        }

        case 'm':
        case 'M':
        {
            if( _modelDist.empty( )) // no models
                return true;

            // current model
            const uint32_t viewID = _frameData.getCurrentViewID();
            View* view = static_cast< View* >( findView( viewID ));
            const uint32_t currentID = view ? 
                view->getModelID() : _frameData.getModelID();

            // next model
            ModelDistVector::const_iterator i;
            for( i = _modelDist.begin(); i != _modelDist.end(); ++i )
            {
                if( (*i)->getID() != currentID )
                    continue;
                
                ++i;
                break;
            }
            if( i == _modelDist.end( ))
                i = _modelDist.begin(); // wrap around
            
            // set identifier on view or frame data (default model)
            const uint32_t modelID = (*i)->getID();
            if( view )
                view->setModelID( modelID );
            else
                _frameData.setModelID( modelID );
            
            if( view )
            {
                const Model* model = getModel( modelID );
                _setMessage( "Model " + eq::base::getFilename( model->getName())
                             + " active" );
            }
            return true;
        }

        case 'l':
        case 'L':
        {
            if( !_currentCanvas )
                return true;

            _frameData.setCurrentViewID( EQ_ID_INVALID );

            uint32_t index = _currentCanvas->getActiveLayoutIndex() + 1;
            const eq::LayoutVector& layouts = _currentCanvas->getLayouts();
            EQASSERT( !layouts.empty( ))

            if( index >= layouts.size( ))
                index = 0;

            _currentCanvas->useLayout( index );
            
            const eq::Layout* layout = _currentCanvas->getLayouts()[index];
            std::ostringstream stream;
            stream << "Layout ";
            if( layout )
            {
                const std::string& name = layout->getName();
                if( name.empty( ))
                    stream << index;
                else
                    stream << name;
            }
            else
                stream << "NONE";
            
            stream << " active";
            _setMessage( stream.str( ));
            return true;
        }

        case 'w':
        case 'W':
            _frameData.toggleWireframe();
            return true;

        case 'r':
        case 'R':
            _frameData.toggleRenderMode();
            return true;

        // Head Tracking Emulation
        case eq::KC_UP:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.y() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_DOWN:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.y() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_RIGHT:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.x() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_LEFT:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.x() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_DOWN:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.z() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_UP:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.z() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '.':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_x( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ',':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_x( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ';':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_y( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '\'':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_y( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '[':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_z( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ']':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_z( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }

        default:
            return false;
    }
}
*/
