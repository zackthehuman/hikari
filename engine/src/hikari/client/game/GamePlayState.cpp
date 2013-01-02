#include <hikari/client/game/GamePlayState.hpp>
#include <hikari/client/game/GameProgress.hpp>
#include <hikari/client/game/GameWorld.hpp>
#include <hikari/client/scripting/SquirrelService.hpp>

#include <hikari/client/game/objects/GameObject.hpp>
#include <hikari/client/game/objects/RockmanHero.hpp>
#include <hikari/client/game/objects/ScriptBrain.hpp>
#include <hikari/client/Services.hpp>

#include <hikari/core/game/map/MapLoader.hpp>
#include <hikari/core/game/map/MapRenderer.hpp>
#include <hikari/core/game/map/Room.hpp>
#include <hikari/core/game/map/RoomTransition.hpp>
#include <hikari/core/gui/ImageFont.hpp>
#include <hikari/core/util/JsonUtils.hpp>
#include <hikari/core/util/PhysFS.hpp>
#include <hikari/core/util/PhysFSUtils.hpp>
#include <hikari/core/util/ServiceLocator.hpp>
#include <hikari/core/util/StringUtils.hpp>

#include <hikari/core/util/Log.hpp>

#include <json/value.h>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>

// #include <oolua.h>

#include <memory>
#include <string>

namespace hikari {

    GamePlayState::GamePlayState(const std::string &name, const Json::Value &params, ServiceLocator &services)
        : name(name)
        , gameProgress(services.locateService<GameProgress>(Services::GAMEPROGRESS))
        , guiFont(services.locateService<ImageFont>(Services::GUIFONT))
        , scriptEnv(services.locateService<SquirrelService>(Services::SCRIPTING))
        , isViewingMenu(false)
        , subState(nullptr)
        , mapRenderer(new MapRenderer(nullptr, nullptr))
        , currentMap(nullptr)
        , currentRoom(nullptr)
        , camera(Rectangle2D<float>(0.0f, 0.0f, 256.0f, 240.0f))
        , world()
    {
        loadMaps(services.locateService<MapLoader>(hikari::Services::MAPLOADER), params);
        /*
        auto go1 = std::make_shared<GameObject>(8905);
        auto go2 = std::make_shared<GameObject>(1001);

        ScriptBrain scriptBrain1(*scriptEnv, "assets/scripts/testbrain.lua");
        scriptBrain1.registerObject(*go1);

        ScriptBrain scriptBrain2(*scriptEnv, "assets/scripts/testbrain.lua");
        scriptBrain2.registerObject(*go2);

        go1->brain = scriptBrain1;
        go2->brain = scriptBrain2;

        world.queueObjectAddition(go1);
        world.queueObjectAddition(go2);
        */

        subState.reset(new ReadySubState(this));
        subState->enter();
    }

    void GamePlayState::handleEvent(sf::Event &event) {
        if((event.type == sf::Event::KeyPressed) && event.key.code == sf::Keyboard::Return) {
            isViewingMenu = !isViewingMenu;
            //scriptEnv->runChunk("print(\"running chunk from game state!\")");
        }

        if((event.type == sf::Event::KeyPressed) && event.key.code == sf::Keyboard::BackSpace) {
            if(subState) {
                changeSubState(std::unique_ptr<SubState>(new ReadySubState(this)));
            }
        }
    }

    void GamePlayState::render(sf::RenderTarget &target) {
        target.clear(sf::Color(255, 0, 255, 255));

        if(subState) {
            subState->render(target);
        }
    }

    bool GamePlayState::update(const float &dt) {
        if(subState) {
            subState->update(dt);
        }

        mapRenderer->setCullRegion(camera.getBoundary());
        
        return false;
    }

    void GamePlayState::onEnter() {
        startStage();

        // Determine which stage we're on and set that to the current level...
        currentMap = maps.at("map-test.json");

        if(currentMap != nullptr) {
            currentRoom = currentMap->getRoom(0);

            if(currentRoom != nullptr) {
                camera.setBoundary(currentRoom->getCameraBounds());
                camera.lockVertical(true);
                camera.lockHorizontal(true);
                camera.lookAt(0, 0);
                mapRenderer->setRoom(currentRoom);
                mapRenderer->setTileData(currentMap->getTileset());
                mapRenderer->setCullRegion(camera.getBoundary());
            }
        }
    }

    void GamePlayState::onExit() { }

    const std::string& GamePlayState::getName() const { 
        return name;
    }

    void GamePlayState::changeSubState(std::unique_ptr<SubState> & newSubState) {
        if(newSubState) {
            if(subState) {
                subState->exit();
            }
            subState = std::move(newSubState);
            subState->enter();
        }
    }

    void GamePlayState::loadMaps(const std::shared_ptr<MapLoader> &mapLoader, const Json::Value &params) {
        try {
            std::string stagesDirectory = params["assets"]["stages"].asString();

            bool directoryExists = PhysFS::exists(stagesDirectory);
            bool directoryIsDirectory = PhysFS::isDirectory(stagesDirectory);

            if(directoryExists && directoryIsDirectory) {
                // Get file listing and load all .json files as maps
                auto fileListing = PhysFS::getFileListing(stagesDirectory);

                HIKARI_LOG(debug) << "Found " << fileListing.size() << " file(s) in map directory.";

                for(auto index = std::begin(fileListing), end = std::end(fileListing); index != end; index++) {
                    const auto & fileName = (*index);
                    const auto & filePath = stagesDirectory + "/" + fileName; // TODO: Handle file paths for real

                    if(StringUtils::endsWith(filePath, ".json")) {
                        try {
                            HIKARI_LOG(debug) << "Loading map from \"" << fileName << "\"...";

                            auto mapJsonObject = JsonUtils::loadJson(filePath);
                            auto map = mapLoader->loadFromJson(mapJsonObject);

                            if(map) {
                                maps[fileName] = map;
                                HIKARI_LOG(debug) << "Successfully loaded map from \"" << fileName << "\".";
                            } else {
                                HIKARI_LOG(error) << "Failed to load map from \"" << filePath << "\".";
                            }
                        } catch(std::exception &ex) {
                            HIKARI_LOG(error) << "Failed to load map from \"" << filePath << "\". Error: " << ex.what();
                        }
                    }
                }
            } else {
                HIKARI_LOG(error) << "Failed to load maps! Specified path doesn't exist or isn't a directory.";
            }
        } catch(std::exception& ex) {
            HIKARI_LOG(error) << "Failed to load maps! Reason: " << ex.what();
        }
    }

    void GamePlayState::startStage() {
        // Start music
        // Show "READY"
        
        // Call playerBirth();
        playerBirth();

        // Show hero
        // Show game objects
        // Enable player controls
    }

    void GamePlayState::restartStage() {

    }

    void GamePlayState::endStage() {

    }

    void GamePlayState::playerBirth() {
        // Teleport Rock to starting point in current room
        // Play "teleported" sample
    }

    void GamePlayState::playerDeath() {
        // Hide hero
        // Spawn explosion energy balls in all directions
        // Stop music
        // Play death sample
    }

    void GamePlayState::checkCollisionWithTransition() { }

    void GamePlayState::renderMap(sf::RenderTarget &target) const {
        auto oldView = target.getView();
        
        target.setView(camera.getPixelAlignedView());
        mapRenderer->render(target);

        target.setView(oldView);
    }

    void GamePlayState::renderEntities(sf::RenderTarget &target) const {
        auto oldView = target.getView();
        
        target.setView(camera.getPixelAlignedView());

        // Render the entities here...

        target.setView(oldView);
    }

    void GamePlayState::renderHud(sf::RenderTarget &target) const {
        //guiFont->renderText(target, "HUD", 72, 32);

        if(isViewingMenu) {
            //guiFont->renderText(target, "MENU", 72, 40);
        }
    }


    // ************************************************************************
    // Definition of sub-states
    // ************************************************************************

    GamePlayState::SubState::SubState(GamePlayState * gamePlayState)
        : gamePlayState(gamePlayState)
    {
    }

    //
    // ReadySubState
    //
    GamePlayState::ReadySubState::ReadySubState(GamePlayState * gamePlayState)
        : SubState(gamePlayState)
        , renderReadyText(false)
        , renderFadeOverlay(true)
        , timer(0.0f)
        , fadeOverlay()
    {
        std::cout << "ReadySubState()" << std::endl;
        if(gamePlayState) {
            fadeOverlay.setSize(
                sf::Vector2f(gamePlayState->camera.getView().getWidth(), gamePlayState->camera.getView().getHeight()));
            
            fadeOverlay.setPosition(0.0f, 0.0f);
            fadeOverlay.setFillColor(sf::Color::Black);
        }
    }

    GamePlayState::ReadySubState::~ReadySubState() {
        std::cout << "~ReadySubState()" << std::endl;
    }

    void GamePlayState::ReadySubState::enter() {
        std::cout << "ReadySubState::enter()" << std::endl;
        timer = 0.0f;

        renderFadeOverlay = true;
        renderReadyText = false;

        sf::Color overlayColor = sf::Color(fadeOverlay.getFillColor());
        overlayColor.a = 0;

        fadeOverlay.setFillColor(overlayColor);
    }

    void GamePlayState::ReadySubState::exit() {
        std::cout << "ReadySubState::exit()" << std::endl;
    }

    void GamePlayState::ReadySubState::update(const float & dt) {
        timer += dt;

        if(timer >= (0.0f * (1.0f/60.0f))) {
            sf::Color overlayColor = sf::Color(fadeOverlay.getFillColor());
            overlayColor.a = 255 - 255 / 4;

            fadeOverlay.setFillColor(overlayColor);
        }

        if(timer >= (5.0f * (1.0f/60.0f))) {
            sf::Color overlayColor = sf::Color(fadeOverlay.getFillColor());
            overlayColor.a = 255 - 255 / 2;

            fadeOverlay.setFillColor(overlayColor);
        }

        if(timer >= (9.0f * (1.0f/60.0f))) {
            sf::Color overlayColor = sf::Color(fadeOverlay.getFillColor());
            overlayColor.a = 255 - ((255 / 4) + (255 / 2));

            fadeOverlay.setFillColor(overlayColor);
        }

        if(timer >= (13.0f * (1.0f/60.0f))) {
            renderFadeOverlay = false;
        }

        if(timer >= (24.0f * (1.0f / 60.0f))) {
            renderReadyText = true;
        }

        if(timer >= ((24.0f + 15.0f) * (1.0f / 60.0f))) {
            renderReadyText = false;
        }

        if(timer >= ((24.0f + 15.0f + 15.0f) * (1.0f / 60.0f))) {
            renderReadyText = true;
        }

        if(timer >= ((24.0f + 15.0f + 15.0f + 15.0f) * (1.0f / 60.0f))) {
            renderReadyText = false;
        }

        if(timer >= ((24.0f + 15.0f + 15.0f + 15.0f + 3.0f) * (1.0f / 60.0f))) {
            renderReadyText = false;
        }

        // The "READY" sequence is 76 frames long, ~1.2666 seconds.
        if(timer >= (76.0f * (1.0f/60.0f))) {
            gamePlayState->changeSubState(std::unique_ptr<SubState>(new PlayingSubState(gamePlayState)));
        }
    }

    void GamePlayState::ReadySubState::render(sf::RenderTarget &target) {
        gamePlayState->renderMap(target);

        if(renderFadeOverlay) {
            target.draw(fadeOverlay);
        }

        if(renderReadyText) {
            gamePlayState->guiFont->renderText(target, "READY", 108, 121);
        }
    }

    //
    // TeleportSubState
    //
    GamePlayState::TeleportSubState::TeleportSubState(GamePlayState * gamePlayState)
        : SubState(gamePlayState)
    {

    }

    GamePlayState::TeleportSubState::~TeleportSubState() {
        
    };

    void GamePlayState::TeleportSubState::enter() {

    }

    void GamePlayState::TeleportSubState::exit() {

    }

    void GamePlayState::TeleportSubState::update(const float & dt) {

    }

    void GamePlayState::TeleportSubState::render(sf::RenderTarget &target) {
        gamePlayState->renderMap(target);
    }

    //
    // PlayingSubState
    //
    GamePlayState::PlayingSubState::PlayingSubState(GamePlayState * gamePlayState)
        : SubState(gamePlayState)
    {
        std::cout << "PlayingSubState()" << std::endl;
    }

    GamePlayState::PlayingSubState::~PlayingSubState() {
        std::cout << "~PlayingSubState()" << std::endl;
    }

    void GamePlayState::PlayingSubState::enter() {
        std::cout << "PlayingSubState::enter()" << std::endl;
    }

    void GamePlayState::PlayingSubState::exit() {
        std::cout << "PlayingSubState::exit()" << std::endl;
    }

    void GamePlayState::PlayingSubState::update(const float & dt) {

    }

    void GamePlayState::PlayingSubState::render(sf::RenderTarget &target) {
        gamePlayState->renderMap(target);
        gamePlayState->renderEntities(target);
        gamePlayState->renderHud(target);

        int line;
        MapIterator index;
        MapIterator end;
        
        for(line = 0, index = gamePlayState->maps.begin(), end = gamePlayState->maps.end(); index != end; index++, line++) {
            gamePlayState->guiFont->renderText(target, (*index).first, 8, ((line * 8) + 48));
        }
    }

    //
    // TransitionSubState
    //
    GamePlayState::TransitionSubState::TransitionSubState(GamePlayState * gamePlayState)
        : SubState(gamePlayState)
    {

    }

    GamePlayState::TransitionSubState::~TransitionSubState() {
    
    }

    void GamePlayState::TransitionSubState::enter() {

    }

    void GamePlayState::TransitionSubState::exit() {

    }

    void GamePlayState::TransitionSubState::update(const float & dt) {

    }

    void GamePlayState::TransitionSubState::render(sf::RenderTarget &target) {

    }

} // hikari