//
//  entry_point.cpp
//  FmodTest_macosx
//
//  Created by Cody Claborn on 12/3/15.
//  Copyright © 2015 oxygine. All rights reserved.
//

#include "core/oxygine.h"
#include "Stage.h"
#include "DebugActor.h"

#include "example.hpp"
#include "AudioEngine.h"

using namespace oxygine;

CAudioEngine aEngine;

//Called each frame
int mainloop() {
    example_update();
    
    getStage()->update();
    
    aEngine.Update();
    
    if(core::beginRendering()) {
        Color clearColor(32,32,32,255);
        Rect viewport(Point(0, 0), core::getDisplaySize());
        getStage()->render(clearColor, viewport);
        
        core::swapDisplayBuffers();
    }
    
    // core update changes when user wants to quit
    bool done = core::update();
    
    return done ? 1: 0;
}

void run() {
    ObjectBase::__startTracingLeaks();
    
    aEngine.Init();
    
    //core oxygine init
    core::init_desc desc;
    
#if OXYGINE_SDL || OXYGINE_EMSCRIPTEN
    // initial size for web and sdl builds
    desc.w = 960;
    desc.h = 640;
#endif
    
    example_preinit();
    core::init(&desc);
    
    //create stage as root node
    Stage::instance = new Stage(true);
    Point size = core::getDisplaySize();
    getStage()->setSize(size);
    
    //Helper debug node
    DebugActor::show();
    
    // init example
    example_init();
    
#ifdef EMSCRIPTEN
    /*
     if you build for Emscripten mainloop would be called automatically outside.
     see emscripten_set_main_loop below
     */
    return;
#endif
    
    while(1) {
        int done = mainloop();
        if (done)
            break;
    }
    //user wants to leave application...
    
    //lets dump all created objects into log
    //all created and not freed resources would be displayed
    ObjectBase::dumpCreatedObjects();
    
    //lets cleanup everything right now and call ObjectBase::dumpObjects() again
    //we need to free all allocated resources and delete all created actors
    //all actors/sprites are smart pointer objects and actually you don't need it remove them by hands
    //but now we want delete it by hands
    
    //check example.cpp
    example_destroy();
    
    
    //renderer.cleanup();
    
    /**releases all internal components and Stage*/
    core::release();
    
    //dump list should be empty now
    //we deleted everything and could be sure that there aren't any memory leaks
    ObjectBase::dumpCreatedObjects();
    
    ObjectBase::__stopTracingLeaks();
    //end
}

#ifdef __S3E__
int main(int argc, char* argv[])
{
    run();
    return 0;
}
#endif


#ifdef OXYGINE_SDL

#include "SDL_main.h"
extern "C"
{
    int main(int argc, char* argv[])
    {
        run();
        return 0;
    }
};
#endif

#ifdef EMSCRIPTEN
#include <emscripten.h>

void one() { mainloop(); }

int main(int argc, char* argv[])
{
    run();
    emscripten_set_main_loop(one, 0, 0);
    return 0;
}
#endif

