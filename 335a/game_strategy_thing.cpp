/*
 *╔═╦═╦═╦╦╦══╦═╦╗─╔╦══╗ 
 *║╦╣║║║║║╠╗╗║╦╣╚╦╝║══╣
 *║╩╣║║║║║╠╩╝║╩╬╗║╔╬══║
 *╚═╩╩═╩╩═╩══╩═╝╚═╝╚══╝
 *       EmuDevs - (http://emudevs.com)
*/
enum NpcIds
{
    NPC_ARMORED_ORC                  = 65000, // Player 1
    NPC_GOSSIP_PLAYER_ONE            = 65001, // Player 1
    NPC_ARMORED_SKELETON             = 65002, // Player 1
    NPC_DEFENSE_TOWER_SPIKE          = 65003, // Player 1
    NPC_GOSSIP_PLAYER_TWO            = 65004, // Player 2
    NPC_PLAYER_TWO_BOSS              = 65005, // Player 2
    NPC_VIKING                       = 65006, // Player 2
    NPC_ARMORED_VIKING               = 65007, // Player 2
    NPC_DRAGON                       = 65008, // Player 2
    NPC_PLAYER_ONE_BOSS              = 65009, // Player 1
    NPC_DEATH_DRAGON                 = 65010, // Player 1
};

struct PlayerData
{
    uint32 resources;
    uint32 score;
    uint8 position;
};

static Position creatureSpawns[] =
{
	/*   PLAYER       NPC        SPAWNS     */
    { -7057.590f, -2098.294f, 391.096893f, 1.586117f }, // MAIN (GOSSIP) 1
    { -7057.604f, -2095.022f, 390.935669f, 1.578219f }, // GOSSIP 1 UNIT SPAWN
    { -7047.958f, -2094.364f, 390.935669f, 2.308640f }, // GOSSIP 1 DEFENSE SPAWN
    { -7064.366f, -2093.279f, 390.935547f, 0.753551f }, // GOSSIP 1 DEFENSE SPAWN 2
    { -7058.394f, -2044.351f, 390.935303f, 4.735566f }, // MAIN (GOSSIP) 2
    { -7058.321f, -2047.497f, 390.935303f, 4.735566f }, // GOSSIP 2 UNIT SPAWN
};

std::map<uint64, PlayerData> PlayerDataContainer;
static int taken = 0;

bool IsPlayerActive(uint64 guid, uint8 pos)
{
    std::map<uint64, PlayerData>::iterator itr = PlayerDataContainer.find(guid);
    if (itr != PlayerDataContainer.end())
        if (itr->second.position != pos)
            return false;
    return true;
}

void IncrementResources(uint32 resources, uint8 pos)
{
    for (std::map<uint64, PlayerData>::const_iterator itr = PlayerDataContainer.begin(); itr != PlayerDataContainer.end(); ++itr)
        if (itr->second.position == pos)
            PlayerDataContainer[itr->first].resources += resources;
}

class npc_player_one : public CreatureScript
{
public:
    npc_player_one() : CreatureScript("npc_player_one") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
		if (taken == 0)
		{
			PlayerDataContainer[player->GetGUID()].position = 1;
			PlayerDataContainer[player->GetGUID()].resources = 20;
			PlayerDataContainer[player->GetGUID()].score = 0;
			ChatHandler(player->GetSession()).SendSysMessage("You have been registered!");
			taken++;
			return false;
		}

        if (IsPlayerActive(player->GetGUID(), 1))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Death Dragon [3]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Armored Orc [2]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Armored Skeleton [1]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Defense Tower", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Boss", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        }
        else
        {
			player->GetSession()->SendNotification("The battle hasn't started or you aren't allowed to gossip with me!");
			player->CLOSE_GOSSIP_MENU();
			return false;
        }
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Nevermind..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+20);
        player->SEND_GOSSIP_MENU(1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
	{
        player->PlayerTalkClass->ClearMenus();
        npc_player_oneAI* gameAI = CAST_AI(npc_player_oneAI, creature->GetAI());

        if (gameAI->Queue.size() == 4) // Don't want too many out there
        {
            player->GetSession()->SendNotification("Exceeded queue size!");
            return false;
        }

		if (actions == GOSSIP_ACTION_INFO_DEF+1)
            gameAI->Queue[NPC_DEATH_DRAGON] = 3;
        else if (actions == GOSSIP_ACTION_INFO_DEF+2)
        {
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+3)
        {
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+4)
        {
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+5)
        {
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+20)
            player->CLOSE_GOSSIP_MENU();
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    struct npc_player_oneAI : public ScriptedAI
    {
        npc_player_oneAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 gameTimer;
		bool hasStarted;
        std::map<uint32, uint16> Queue; // [Entry] - [Size]

		void Reset()
		{
			gameTimer = 1000;
			hasStarted = false;
		}

        void UpdateAI(uint32 diff)
        {
            if (gameTimer <= diff)
			{
				if (PlayerDataContainer.size() == 2)
                    SpawnCreatureInQueue(); // Will spawn a random creature in the queue
                gameTimer = urand(6000, 13000);
			}
            else
                gameTimer -= diff;
        }

        void UpdateCreatureQueueSize(uint32 entry)
        {
            for (std::map<uint32, uint16>::iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
                if (itr->first == entry)
                    itr->second -= 1;
            DeleteCreatureFromQueue(); // Call this after it has been updated
        }

        // Deletes a creature from the queue once the size reaches 0
        void DeleteCreatureFromQueue()
        {
            for (std::map<uint32, uint16>::const_iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
                if (itr->second == 0)
                    Queue.erase(itr);
        }

        // Spawns a random unit from the queue
        void SpawnCreatureInQueue()
        {
			int i = 0;
			if (Queue.size() > 0)
			{
				for (std::map<uint32, uint16>::const_iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
				{
					if (i > 0)
						continue;

					me->SummonCreature(itr->first, creatureSpawns[1].GetPositionX(), creatureSpawns[1].GetPositionY(), creatureSpawns[1].GetPositionZ(), creatureSpawns[1].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
				    UpdateCreatureQueueSize(itr->first);
					i++;
				}
			}
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_player_oneAI(creature);
    }
};

class npc_player_two : public CreatureScript
{
public:
	npc_player_two() : CreatureScript("npc_player_two") { }

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (taken == 1) // If player one started, player 2 can start
		{
			PlayerDataContainer[player->GetGUID()].position = 2;
			PlayerDataContainer[player->GetGUID()].resources = 20;
			PlayerDataContainer[player->GetGUID()].score = 0;
			ChatHandler(player->GetSession()).SendSysMessage("You have been registered!");
			player->CLOSE_GOSSIP_MENU();
			taken++;
			return false;
		}
		else if (taken == 2)
		{
		    if (IsPlayerActive(player->GetGUID(), 2))
		    {
			    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Viking [4]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
			    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Armored Viking [3]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
			    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Mini Dragon [2]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
			    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Boss", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
		    }
		    else
		    {
			    player->GetSession()->SendNotification("The battle hasn't started or you aren't allowed to gossip with me!");
			    return false;
		    }
		}
		else
		{
			player->GetSession()->SendNotification("You must wait for player 1!");
			return false;
		}
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Nevermind..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+20);
		player->SEND_GOSSIP_MENU(1, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
	{
		player->PlayerTalkClass->ClearMenus();
        npc_player_twoAI* gameAI = CAST_AI(npc_player_twoAI, creature->GetAI());

        if (gameAI->Queue.size() == 5) // Don't want too many out there
        {
            player->GetSession()->SendNotification("Exceeded queue size!");
            return false;
        }
		return true;
	}

	struct npc_player_twoAI : public ScriptedAI
	{
		npc_player_twoAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 gameTimer;
		std::map<uint32, uint16> Queue; // [Entry] - [Size]

		void Reset()
		{
			gameTimer = urand(6000, 13000);
		}

        void UpdateAI(uint32 diff)
        {
            if (PlayerDataContainer.size() < 2)
                return;
            else
                gameTimer = urand(6000, 13000);

            if (gameTimer <= diff)
                SpawnCreatureInQueue(); // Will spawn a random creature in the queue
            else
                gameTimer -= diff;
        }

        void UpdateCreatureQueueSize(uint32 entry)
        {
            for (std::map<uint32, uint16>::iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
                if (itr->first == entry)
                    itr->second -= 1;
            DeleteCreatureFromQueue(); // Call this after it has been updated
        }

        // Deletes a creature from the queue once the size reaches 0
        void DeleteCreatureFromQueue()
        {
            for (std::map<uint32, uint16>::const_iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
                if (itr->second <= 0)
                    Queue.erase(itr);
        }

        // Spawns a random unit from the queue
        void SpawnCreatureInQueue()
        {
			int i = 0;
			if (Queue.size() > 0)
			{
				for (std::map<uint32, uint16>::const_iterator itr = Queue.begin(); itr != Queue.end(); ++itr)
				{
					if (i > 0)
						continue;

					me->SummonCreature(itr->first, creatureSpawns[1].GetPositionX(), creatureSpawns[1].GetPositionY(), creatureSpawns[1].GetPositionZ(), creatureSpawns[1].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
				    UpdateCreatureQueueSize(itr->first);
					i++;
				}
			}
        }
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_player_twoAI(creature);
	}
};

class npc_player_one_units : public CreatureScript
{
public:
	npc_player_one_units() : CreatureScript("npc_player_one_units") { }

	struct npc_player_one_unitsAI : public ScriptedAI
	{
		npc_player_one_unitsAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 moveTimer;
		uint32 enemyNearTimer;

		void Reset()
		{
			moveTimer = 1000;
			enemyNearTimer = 1000;

			if (me->GetEntry() == NPC_DEFENSE_TOWER_SPIKE)
				SetCombatMovement(false);
		}

		void UpdateAI(uint32 diff)
		{
			if (!UpdateVictim())
				me->GetMotionMaster()->MovePoint(0, creatureSpawns[4].GetPositionX(), creatureSpawns[4].GetPositionY(), creatureSpawns[4].GetPositionZ());
			else
				me->GetMotionMaster()->Clear();

			if (enemyNearTimer <= diff)
			{
				if (Unit* unit = me->FindNearestCreature(NPC_GOSSIP_PLAYER_TWO, 10.0f, true))
					me->GetAI()->AttackStart(unit);
				enemyNearTimer = 1000;
			}
			else
				enemyNearTimer -= diff;
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_player_one_unitsAI(creature);
	}
};

class npc_player_two_units : public CreatureScript
{
public:
	npc_player_two_units() : CreatureScript("npc_player_two_units") { }

	struct npc_player_two_unitsAI : public ScriptedAI
	{
		npc_player_two_unitsAI(Creature* creature) : ScriptedAI(creature) { }

		void Reset()
		{
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_player_two_unitsAI(creature);
	}
};

void AddSC_game_thing()
{
	new npc_player_one;
	new npc_player_two;
	new npc_player_one_units;
	new npc_player_two_units;
}