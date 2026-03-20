using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.MeshRenderer;
using static Engine.Event;

namespace Game
{
    public class WallManager : ScriptBehaviour
    {
        //==================== Wall Settings ==============================
        [SerializeField] private const string wall1Name = "Wall1";
        [SerializeField] private const string wall2Name = "Wall2";
        [SerializeField] private const string wall3Name = "Wall3";
        //[SerializeField] private const string wall4Name = "Wall1";
        private uint wall1ID = 0;
        private uint wall2ID = 0;
        private uint wall3ID = 0;
        //private uint wall4ID = 0;

        //================== EVENT NAMES ================================
        //private const string EVENT_1MIN = "1MIN";
        private const string EVENT_2MIN = "2MIN";
        private const string EVENT_WALLENABLED = "WallEnabled";
        private const string EVENT_GAMEOVER = "GameOver";
        private const string EVENT_GAMEWIN = "GameWin";

        //=================== STATE ============================
        private bool initialized = false;
        private bool active = false;

        public override void OnStart(){

            initialize();
            
            Subscribe(EVENT_GAMEOVER, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);
            
            //Subscribe(EVENT_1MIN, OnOneMin);
            Subscribe(EVENT_2MIN, OnTwoMin);

        }

        public override void OnUpdate(float deltaTime){

            if(GameState.IsPaused){
                return;
            }
        }

        public override void OnDestroy(){
            Unsubscribe(EVENT_GAMEOVER, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);
            //Unsubscribe(EVENT_1MIN, OnOneMin);
            Unsubscribe(EVENT_2MIN, OnTwoMin);
        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[WallManager] Detect Game End");
            active = false;
        }

        // private void OnOneMin(string eventName, string payload){
        //     SetWallActive(wall2ID);
        //     SetWallActive(wall3ID);
        // }

        private void OnTwoMin(string eventName, string payload){
            //SetWallActive(wall4ID);
            SetWallActive(wall2ID);
            SetWallActive(wall3ID);
        }

        private void initialize(){

            wall1ID = SceneFindEntityByName(wall1Name);
            wall2ID = SceneFindEntityByName(wall2Name);
            wall3ID = SceneFindEntityByName(wall3Name);
            //wall4ID = SceneFindEntityByName(wall4Name);

            if(wall1ID == 0){
                LogMessage("[WallManaager] Warning Wall1ID not found");
            }
            if(wall2ID == 0){
                LogMessage("[WallManaager] Warning Wall2ID not found");
            }
            if(wall3ID == 0){
                LogMessage("[WallManaager] Warning Wall3ID not found");
            }
            // if(wall4ID == 0){
            //     LogMessage("[WallManaager] Warning Wall4ID not found");
            // }

            SetWallActive(wall1ID);

            active = true;
            initialized = true;
        }

        private void SetWallActive(uint wallID){
            if(wallID == 0){
                return;
            }

            SetVisible(wallID, true); //set the wall to be visible to enable the spawning from that particular wall
            Publish(EVENT_WALLENABLED, ""); //Publish event for all wall to check themselves
        }
    }
}