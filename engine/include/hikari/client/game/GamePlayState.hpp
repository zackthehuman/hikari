#ifndef HIKARI_CLIENT_GAME_GAMEPLAYSTATE
#define HIKARI_CLIENT_GAME_GAMEPLAYSTATE

#include <hikari/core/game/GameState.hpp>
#include <hikari/core/game/map/Camera.hpp>
#include <hikari/core/game/map/Map.hpp>

#include <hikari/client/game/GameWorld.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <memory>

#include <string>
#include <map>


// #include <oolua.h>

namespace Json {
    class Value;
}

namespace hikari {

    class EnergyMeter;
    class GameProgress;
    class ImageFont;
    class MapLoader;
    class ServiceLocator;
    class SquirrelService;
    class Room;
    class Map;
    class MapRenderer;

    class GamePlayState : public GameState {

    class SubState;

    private:
        std::string name;
        std::shared_ptr<GameProgress> gameProgress;
        std::shared_ptr<ImageFont> guiFont;
        std::shared_ptr<SquirrelService> scriptEnv;
        std::map< std::string, std::shared_ptr<Map> > maps;
        typedef std::map< std::string, std::shared_ptr<Map> >::iterator MapIterator;

        std::shared_ptr<Map> currentMap;
        std::shared_ptr<Room> currentRoom;

        GameWorld world;
        Camera camera;
        std::unique_ptr<MapRenderer> mapRenderer;

        // Gui
        std::shared_ptr<EnergyMeter> hudPlayerEnergyMeter;
        std::shared_ptr<EnergyMeter> hudCurrentWeaponMeter;
        sf::View view;

        // Gameplay Flags
        bool isViewingMenu;

        //
        // Resource Management
        //
        void loadMaps(const std::shared_ptr<MapLoader> &mapLoader, const Json::Value &params);

        //
        // Gameplay Mechanics
        //
        std::unique_ptr<SubState> subState;
        void changeSubState(std::unique_ptr<SubState> & newSubState);

        /**
            Starts the current stage from the beginning.
        */
        void startStage();

        /**
            Restarts the current stage from the closest restart point.
        */
        void restartStage();

        /**
            Handles actions that take place after a stage has been completed.
        */
        void endStage();

        void playerBirth();
        void playerDeath();

        /**
            Checks if the player is colliding with a transition region.
            If it is then some kind of flag needs to be set to begin a transition.
        */
        void checkCollisionWithTransition();

        //
        // Rendering
        //
        void renderMap(sf::RenderTarget &target) const;
        void renderEntities(sf::RenderTarget &target) const;
        void renderHud(sf::RenderTarget &target) const;
    
        /**
         * GamePlayState::SubState encapsulates a part of gameplay that operates
         * independently from others. Some examples of this would be:
         *  - Starting a stage ("READY")
         *  - Teleporting megaman in from the top of the screen
         *  - Actually plasying a stage
         *  - Transitioning from one room to another (automatic camera movement)
         *  - Ending a stage (success or death)
         *  - Pausing / menu screen
         */
        class SubState {
        protected:
            GamePlayState * gamePlayState;
            SubState(GamePlayState * gamePlayState);
        public:
            virtual ~SubState() { };
            virtual void enter() = 0;
            virtual void exit() = 0;
            virtual void update(const float & dt) = 0;
            virtual void render(sf::RenderTarget &target) = 0;
        };

        /**
         * Shows the "READY" animation and then starts the level.
         */
        class ReadySubState : public SubState {
        private:
            bool renderReadyText;
            bool renderFadeOverlay;
            float timer;
            sf::RectangleShape fadeOverlay;

        public:
            ReadySubState(GamePlayState * gamePlayState);
            virtual ~ReadySubState();
            virtual void enter();
            virtual void exit();
            virtual void update(const float & dt);
            virtual void render(sf::RenderTarget &target);
        };

        /**
         * Teleporting sequence, in our out of the screen.
         */
        class TeleportSubState : public SubState {
        public:
            TeleportSubState(GamePlayState * gamePlayState);
            virtual ~TeleportSubState();
            virtual void enter();
            virtual void exit();
            virtual void update(const float & dt);
            virtual void render(sf::RenderTarget &target);
        };

        /**
         * Actual game play, player-controlled movements, etc.
         */
        class PlayingSubState : public SubState {
        public:
            PlayingSubState(GamePlayState * gamePlayState);
            virtual ~PlayingSubState();
            virtual void enter();
            virtual void exit();
            virtual void update(const float & dt);
            virtual void render(sf::RenderTarget &target);
        };

        /**
         * Sequence where hero transitions between rooms.
         */
        class TransitionSubState : public SubState {
        public:
            TransitionSubState(GamePlayState * gamePlayState);
            virtual ~TransitionSubState();
            virtual void enter();
            virtual void exit();
            virtual void update(const float & dt);
            virtual void render(sf::RenderTarget &target);
        };

    public:
        GamePlayState(const std::string &name, const Json::Value &params, ServiceLocator &services);
        virtual ~GamePlayState() { std::cout << "~GamePlayState()" << std::endl; }

        virtual void handleEvent(sf::Event &event);
        virtual void render(sf::RenderTarget &target);
        virtual bool update(const float &dt);
        virtual void onEnter();
        virtual void onExit();
        virtual const std::string &getName() const;
    };

} // hikari

#endif // HIKARI_CLIENT_GAME_GAMEPLAYSTATE