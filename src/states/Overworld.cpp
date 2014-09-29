#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <iostream>
#include <vector>

// boil2d
#include <GameState.hpp>
#include <Functions.hpp>
#include <Overworld.hpp>
#include <App.hpp>
#include <Player.hpp>
#include <DebugDraw.hpp>



OverWorld::OverWorld(int prevState, App* app)
{

    // load the background

    // set starting points based on previous state


    // ground shape
    grnd.setPosition(0,app->window_height-20);
    grnd.setSize(sf::Vector2f(app->window_width,20));
    grnd.setFillColor(sf::Color(76,67,53,128));

    //box2d world stuff
    b2Vec2 gravity(0, -9.8); // earth gravity is -9.8
    //b2Vec2 gravity(0, 0); // earth gravity is -9.8
    world = new b2World(gravity); // second bool sleep argument defaults to true in 2.2.1+
    world->SetAutoClearForces(false);

    // box2d ground body stuff
    sf::Vector2f gb_pos = pixels_to_meters(0,-(app->window_height-20));
    groundBodyDef.position.Set(gb_pos.x, gb_pos.y);
    b2Body* groundBody = world->CreateBody(&groundBodyDef);

    sf::Vector2f gb_scale = pixels_to_meters(app->window_width,-1);
    groundBox.SetAsBox(gb_scale.x, gb_scale.y);
    groundBody->CreateFixture(&groundBox, 0.0f);

    // add player to box2d world
    player.player_body = world->CreateBody(&player.player_bodyDef);
    player.player_body->CreateFixture(&player.player_fixtureDef);

    // debug draw
    debug_draw.LinkTarget(app->window);
    debug_draw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit | b2Draw::e_aabbBit | b2Draw::e_jointBit | b2Draw::e_pairBit);
    world->SetDebugDraw(&debug_draw);


    // box2d timestep values
    timeStep = 1.0f / 60.0f; 
    velocityIterations = 8;
    positionIterations = 3;

}



OverWorld::~OverWorld()
{
    // free resources
    delete world;
}

void OverWorld::pause()
{
    // pause the game
}

void OverWorld::resume()
{
    // pause the game
}

void OverWorld::handle_events(App *app)
{

    //std::cout << "ow handle_events got called" << std::endl;

    sf::Event event;
    while (app->window.pollEvent(event))
    {
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
        {
            set_next_state(STATE_PAUSE);
            //std::cout << "overworld event loop: setting next state to PAUSE" << std::endl;
        }
        
        if(event.type == sf::Event::Closed)
        {
            std::cout << "Got close window event" << std::endl;
            app->window.close();
        }
        
        if (event.type == sf::Event::Resized)
        {
            // this fucks up the object/window scaling
            //app->window_width  = event.size.width;
            //app->window_height = event.size.height;
        }

        /*
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Right))
        {
            //player.player_body->ApplyTorque(float32(-5), true);
            
            //b2Vec2 force = b2Vec2(1.0f, 0);
            //b2Vec2 point = player.player_body->GetWorldCenter();
            //player.player_body->ApplyLinearImpulse(force,point,true);
            player.movement = player.RIGHT;
        }
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Left))
        {
            //player.player_body->ApplyTorque(float32(5), true);
            
            //b2Vec2 force = b2Vec2(-1.0f, 0);
            //b2Vec2 point = player.player_body->GetWorldCenter();
            //player.player_body->ApplyLinearImpulse(force,point,true);
            player.movement = player.LEFT;
        }
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Up))
        {
            player.movement = player.UP;
        }
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Down))
        {
            player.movement = player.DOWN;
        }
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space))
        {
            //player.player_body->ApplyTorque(float32(5), true);
            
            //b2Vec2 force = b2Vec2(-1.0f, 0);
            //b2Vec2 point = player.player_body->GetWorldCenter();
            //player.player_body->ApplyLinearImpulse(force,point,true);
            player.movement = player.STOP;
        }
        */

        // toggle debug draw
        if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F1))
        {
            switch(app->debug_draw) 
            {
                case false:
                    app->debug_draw = true;
                    break;
                case true:
                    app->debug_draw = false;
                    break;
            }
                    

        }

    } // event loop

    // player movement real-time key presses
    player.movement = player.STOP;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        player.movement = player.RIGHT;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        player.movement = player.LEFT;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        player.movement = player.UP;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        player.movement = player.DOWN;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        player.movement = player.STOP;
    }
}

void OverWorld::logic(App* app)
{
    // get previous player position
    player.prev_position = meters_to_pixels(player.player_body->GetPosition().x, player.player_body->GetPosition().y);

    // handle player movement
    b2Vec2 vel = player.player_body->GetLinearVelocity();
    sf::Vector2f desired_vel;
    //float desired_vel_x = 0;
    //float desired_vel_y = 0;
    switch(player.movement)
    {
        case player.LEFT:
            desired_vel.x += b2Max(vel.x - 0.9f, -12.0f);
            break;
        case player.RIGHT:
            desired_vel.x += b2Min(vel.x + 0.9f, 12.0f);
            break;
        case player.UP:
            desired_vel.y += b2Min(vel.y + 0.9f, 12.0f);
            break;
        case player.DOWN:
            desired_vel.y += b2Max(vel.y - 0.9f, -12.0f);
            break;
        case player.STOP:
            desired_vel.x = vel.x * 0.82; // <1, smaller value = faster stop
            desired_vel.y = vel.y * 0.82; // <1, smaller value = faster stop
            break;
    }
    float vel_change_x = desired_vel.x - vel.x;
    float vel_change_y = desired_vel.y - vel.y;
    float impulse_x = player.player_body->GetMass() * vel_change_x;
    float impulse_y = player.player_body->GetMass() * vel_change_y;
    player.player_body->ApplyLinearImpulse(b2Vec2(impulse_x,impulse_y), player.player_body->GetWorldCenter(), true);
    
    // do physics step
    world->Step(timeStep, velocityIterations, positionIterations);

    // get new player position
    player.cur_position = meters_to_pixels(player.player_body->GetPosition().x, player.player_body->GetPosition().y);

    //player.player_shape.setRotation(player_body->GetAngle() * (180/3.14159265359));
    player.update_angle();

    // do logic, collision checks, etc

    // move the player etc

    //std::cout << "inside ow logic loop" << std::endl;
}

void OverWorld::render(App *app, double& alpha)
{
    //std::cout << "ow render got called" << std::endl;
    // clear screen and box2d force cache
    world->ClearForces();
    app->window.clear();

    // draw the ground body
    app->window.draw(grnd);

    // get interpolated position
    float pos_x = player.cur_position.x * alpha + player.prev_position.x * (1.0f - alpha);
    float pos_y = player.cur_position.y * alpha + player.prev_position.y * (1.0f - alpha);

    // position and draw the player
    player.player_shape.setPosition(pos_x, -pos_y);
    app->window.draw(player.player_shape);

    // debug draw if option enabled
    if(app->debug_draw)
        world->DrawDebugData();
}
