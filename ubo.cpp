#include "framework.h"

using namespace prototyper;

int main( int argc, char** argv )
{
  map<string, string> args;

  for( int c = 1; c < argc; ++c )
  {
    args[argv[c]] = c + 1 < argc ? argv[c + 1] : "";
    ++c;
  }

  cout << "Arguments: " << endl;
  for_each( args.begin(), args.end(), []( pair<string, string> p )
  {
    cout << p.first << " " << p.second << endl;
  } );

  uvec2 screen( 0 );
  bool fullscreen = false;
  bool silent = false;
  string title = "Basic CPP to get started";

  /*
   * Process program arguments
   */

  stringstream ss;
  ss.str( args["--screenx"] );
  ss >> screen.x;
  ss.clear();
  ss.str( args["--screeny"] );
  ss >> screen.y;
  ss.clear();

  if( screen.x == 0 )
  {
    screen.x = 1280;
  }

  if( screen.y == 0 )
  {
    screen.y = 720;
  }

  try
  {
    args.at( "--fullscreen" );
    fullscreen = true;
  }
  catch( ... ) {}

  try
  {
    args.at( "--help" );
    cout << title << ", written by Marton Tamas." << endl <<
         "Usage: --silent      //don't display FPS info in the terminal" << endl <<
         "       --screenx num //set screen width (default:1280)" << endl <<
         "       --screeny num //set screen height (default:720)" << endl <<
         "       --fullscreen  //set fullscreen, windowed by default" << endl <<
         "       --help        //display this information" << endl;
    return 0;
  }
  catch( ... ) {}

  try
  {
    args.at( "--silent" );
    silent = true;
  }
  catch( ... ) {}

  /*
   * Initialize the OpenGL context
   */

  framework frm;
  frm.init( screen, title, fullscreen );

  //set opengl settings
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );

  frm.get_opengl_error();

  /*
   * Set up mymath
   */

  camera<float> cam;
  frame<float> the_frame;

  float cam_fov = 45.0f;
  float cam_near = 1.0f;
  float cam_far = 100.0f;

  the_frame.set_perspective( radians( cam_fov ), ( float )screen.x / ( float )screen.y, cam_near, cam_far );

  glViewport( 0, 0, screen.x, screen.y );

  /*
   * Set up the scene
   */

  float move_amount = 5;
  float cam_rotation_amount = 5.0;

  GLuint box = frm.create_box();

  struct trans_prop
  {
    mat4 modelview;
    mat4 proj;
  } trans_ubo;

  GLuint ubo;
  glGenBuffers( 1, &ubo );
  glBindBuffer( GL_UNIFORM_BUFFER, ubo );
  glBufferData( GL_UNIFORM_BUFFER, sizeof( trans_prop ), 0, GL_DYNAMIC_DRAW );

  unsigned int binding_loc = 0;

  /*
   * Set up the shaders
   */

  GLuint debug_shader = 0;
  frm.load_shader( debug_shader, GL_VERTEX_SHADER, "../shaders/ubo/debug.vs" );
  frm.load_shader( debug_shader, GL_FRAGMENT_SHADER, "../shaders/ubo/debug.ps" );

  GLint debug_mvp_mat_loc = glGetUniformLocation( debug_shader, "mvp" );

  GLint index_loc = glGetUniformBlockIndex( debug_shader, "trans_prop" );
  glUniformBlockBinding( debug_shader, index_loc, binding_loc );

  /*
   * Handle events
   */

  auto event_handler = [&]( const sf::Event & ev )
  {
    switch( ev.type )
    {
      case sf::Event::KeyPressed:
        {
          if( ev.key.code == sf::Keyboard::A )
          {
            cam.rotate_y( radians( cam_rotation_amount ) );
          }

          if( ev.key.code == sf::Keyboard::D )
          {
            cam.rotate_y( radians( -cam_rotation_amount ) );
          }

          if( ev.key.code == sf::Keyboard::W )
          {
            cam.move_forward( move_amount );
          }

          if( ev.key.code == sf::Keyboard::S )
          {
            cam.move_forward( -move_amount );
          }
        }
      default:
        break;
    }
  };

  /*
   * Render
   */

  sf::Clock timer;
  timer.restart();

  frm.display( [&]
  {
    frm.handle_events( event_handler );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( debug_shader );

    mat4 view = cam.get_matrix();
    mat4 model = mat4::identity;
    mat4 projection = the_frame.projection_matrix;
    mat4 trans = create_translation( vec3( 0, 0, -5 ) );
    mat4 mv = view * model * trans;
    //glUniformMatrix4fv( debug_mvp_mat_loc, 1, false, &ppl.get_model_view_projection_matrix( cam )[0][0] );
    trans_ubo.modelview = mv;
    trans_ubo.proj = projection;
    glBindBuffer( GL_UNIFORM_BUFFER, ubo );
    glBufferData( GL_UNIFORM_BUFFER, sizeof( trans_prop ), &trans_ubo, GL_DYNAMIC_DRAW );
    glBindBufferBase( GL_UNIFORM_BUFFER, binding_loc, ubo );

    glBindVertexArray( box );
    glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );

    frm.get_opengl_error();
  }, silent );

  return 0;
}
