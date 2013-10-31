/*
 *╔═╦═╦═╦╦╦══╦═╦╗─╔╦══╗ 
 *║╦╣║║║║║╠╗╗║╦╣╚╦╝║══╣
 *║╩╣║║║║║╠╩╝║╩╬╗║╔╬══║
 *╚═╩╩═╩╩═╩══╩═╝╚═╝╚══╝
 *       EmuDevs - (http://emudevs.com)
*/
enum NpcIds
{
    NPC_UNDEAD_BEAST                 = 65000, // Player 1
    NPC_GOSSIP_PLAYER_ONE            = 65001, // Player 1
    NPC_UNDEAD_TROLL                 = 65002, // Player 1
    NPC_DEFENSE_TOWER_SPIKE          = 65003, // Player 1
    NPC_GOSSIP_PLAYER_TWO            = 65004, // Player 2
    NPC_PLAYER_TWO_BOSS              = 65005, // Player 2
    NPC_VIKING                       = 65006, // Player 2
    NPC_ARMORED_VIKING               = 65007, // Player 2
    NPC_DRAGON                       = 65008, // Player 2
    NPC_PLAYER_ONE_BOSS              = 65009, // Player 1
    NPC_UNDEAD_CRYPTO                = 65010, // Player 1
    NPC_TOTEM_TOWER                  = 65011  // Player 2
};

enum SpellIds
{
    SPELL_RENEW                      = 37260,
    SPELL_SHADOW_BOLT                = 49084,
    SPELL_ICE_SPIKE                  = 57454,
    SPELL_HOLY_BOLT                  = 31759,
    SPELL_LIGHTNING_BOLT             = 36152,
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
    { -7065.684f, -2073.248f, 390.936493f, 0.095714f }, // PLAYER ATTACKED MINION(S)
};

std::map<uint64, PlayerData> PlayerDataContainer;
static int taken = 0;
static bool eventOver = true;

bool IsPlayerActive(uint64 guid, uint8 pos)
{
    std::map<uint64, PlayerData>::iterator itr = PlayerDataContainer.find(guid);
    if (itr != PlayerDataContainer.end())
        if (itr->second.position != pos)
            return false;
    return true;
}

void IncrementStats(uint32 resources, uint32 score, uint8 pos)
{
    for (std::map<uint64, PlayerData>::const_iterator itr = PlayerDataContainer.begin(); itr != PlayerDataContainer.end(); ++itr)
        if (itr->second.position == pos)
        {
            PlayerDataContainer[itr->first].score += score;
            PlayerDataContainer[itr->first].resources += resources;
        }
}

class npc_player_one : public CreatureScript
{
public:
    npc_player_one() : CreatureScript("npc_player_one") { }

    bool OnGossipHello(Player* player, Creature* creature) OVERRIDE
    {
        if (taken == 0)
        {
            PlayerDataContainer[player->GetGUID()].position = 1;
            PlayerDataContainer[player->GetGUID()].resources = 20;
            PlayerDataContainer[player->GetGUID()].score = 0;
            creature->setFaction(player->getFaction());
            ChatHandler(player->GetSession()).SendSysMessage("You have been registered!");
            taken++;
            return false;
        }

        if (IsPlayerActive(player->GetGUID(), 1))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Undead Beast [2][10R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Undead Troll [2][25R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Undead Crypto [1][40R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Defense Tower[5R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Boss[100R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            std::map<uint64, PlayerData>::iterator itr = PlayerDataContainer.find(player->GetGUID());
            if (itr != PlayerDataContainer.end())
            {
                char msg [25];
                snprintf(msg, 25, "Resources: %u \n Score: %u", itr->second.resources, itr->second.score);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, msg, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+101);
            }
        }
        else
        {
            player->GetSession()->SendNotification("The battle hasn't started or you aren't allowed to gossip with me!");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Nevermind..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+99);
        player->SEND_GOSSIP_MENU(1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions) OVERRIDE
    {
        player->PlayerTalkClass->ClearMenus();
        npc_player_oneAI* gameAI = CAST_AI(npc_player_oneAI, creature->GetAI());

        if (gameAI->Queue.size() == 3) // Don't want too many out there
        {
            player->GetSession()->SendNotification("Exceeded queue size!");
            return false;
        }

        if (actions == GOSSIP_ACTION_INFO_DEF+1)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 10)
            {
                gameAI->Queue[NPC_UNDEAD_BEAST] = 2;
                PlayerDataContainer[player->GetGUID()].resources -= 10;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+2)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 25)
            {
                gameAI->Queue[NPC_UNDEAD_TROLL] = 2;
                PlayerDataContainer[player->GetGUID()].resources -= 25;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+3)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 40)
            {
                gameAI->Queue[NPC_UNDEAD_CRYPTO] = 1;
                PlayerDataContainer[player->GetGUID()].resources -= 40;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+4)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 5)
            {
                gameAI->Queue[NPC_DEFENSE_TOWER_SPIKE] = 1;
                PlayerDataContainer[player->GetGUID()].resources -= 5;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+5)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 100)
            {
                gameAI->Queue[NPC_PLAYER_ONE_BOSS] = 1;
                PlayerDataContainer[player->GetGUID()].resources -= 100;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+99)
            player->CLOSE_GOSSIP_MENU();
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    struct npc_player_oneAI : public ScriptedAI
    {
        npc_player_oneAI(Creature* creature) : ScriptedAI(creature), summons(me) { }

        uint32 gameTimer;
        uint32 shadowBoltTimer;
        std::map<uint32, uint16> Queue; // [Entry] - [Size]

        void Reset() OVERRIDE
        {
            gameTimer = 1000;
            shadowBoltTimer = 4000;
            summons.DespawnAll();
        }

        void DamageTaken(Unit* attacker, uint32 &damage) OVERRIDE
        {
            if (attacker->GetTypeId() == TYPEID_PLAYER)
            {
                ChatHandler(attacker->ToPlayer()->GetSession()).SendSysMessage("You cannot attack Player One's minion!");
                DoCast(me, SPELL_RENEW, false);
                attacker->ToPlayer()->TeleportTo(0, creatureSpawns[6].GetPositionX(), creatureSpawns[6].GetPositionY(), creatureSpawns[6].GetPositionZ(), creatureSpawns[6].GetOrientation());
            }
        }

        void JustDied(Unit* /*killer*/) OVERRIDE
        {
            summons.DespawnAll();
            me->setFaction(35);
            eventOver = true;
            PlayerDataContainer.clear();
            taken = 0;
        }

        void JustSummoned(Creature* summoned)
        {
            summons.Summon(summoned);
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (gameTimer <= diff)
            {
                if (me->IsAlive() && eventOver)
                    summons.DespawnAll();
                if (PlayerDataContainer.size() == 2)
                    SpawnCreatureInQueue(); // Will spawn a random creature in the queue
                gameTimer = urand(6000, 13000);
            }
            else
                gameTimer -= diff;
            if (!UpdateVictim())
                return;

            if (shadowBoltTimer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_SHADOW_BOLT, true);
                shadowBoltTimer = 4000;
            }
            else
                shadowBoltTimer -= diff;
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

                    me->SummonCreature(itr->first, creatureSpawns[1].GetPositionX(), creatureSpawns[1].GetPositionY() + 5, creatureSpawns[1].GetPositionZ(), creatureSpawns[1].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN)->setFaction(me->getFaction());
                    UpdateCreatureQueueSize(itr->first);
                    i++;
                }
            }
        }
    private:
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_player_oneAI(creature);
    }
};

class npc_player_one_units : public CreatureScript
{
public:
	npc_player_one_units() : CreatureScript("npc_player_one_units") { }

    struct npc_player_one_unitsAI : public ScriptedAI
    {
        npc_player_one_unitsAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() OVERRIDE
        {
        }

        void JustDied(Unit* /*killer*/) OVERRIDE
        {
            IncrementStats(urand(5, 20), urand(5, 15), 2);
        }

        void DamageTaken(Unit* attacker, uint32 &damage) OVERRIDE
        {
            if (attacker->GetTypeId() == TYPEID_PLAYER)
            {
                ChatHandler(attacker->ToPlayer()->GetSession()).SendSysMessage("You cannot attack Player One's minion!");
                DoCast(me, SPELL_RENEW, false);
                me->AI()->Reset();
                me->AttackStop();
                me->SetTarget(0);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, creatureSpawns[4].GetPositionX(), creatureSpawns[4].GetPositionY() - 5, creatureSpawns[4].GetPositionZ());
                attacker->ToPlayer()->TeleportTo(0, creatureSpawns[6].GetPositionX(), creatureSpawns[6].GetPositionY(), creatureSpawns[6].GetPositionZ(), creatureSpawns[6].GetOrientation());
            }
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (!UpdateVictim())
                me->GetMotionMaster()->MovePoint(0, creatureSpawns[4].GetPositionX(), creatureSpawns[4].GetPositionY(), creatureSpawns[4].GetPositionZ());
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_player_one_unitsAI(creature);
    }
};

class npc_player_two : public CreatureScript
{
public:
    npc_player_two() : CreatureScript("npc_player_two") { }

    bool OnGossipHello(Player* player, Creature* creature) OVERRIDE
    {
        if (taken == 1) // If player one started, player 2 can start
        {
            PlayerDataContainer[player->GetGUID()].position = 2;
            PlayerDataContainer[player->GetGUID()].resources = 20;
            PlayerDataContainer[player->GetGUID()].score = 0;
            creature->setFaction(player->getFaction());
            ChatHandler(player->GetSession()).SendSysMessage("You have been registered!");
            player->CLOSE_GOSSIP_MENU();
            taken++;
            eventOver = false;
            return false;
        }
        else if (taken == 2)
        {
            if (IsPlayerActive(player->GetGUID(), 2))
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Viking [3][10R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Armored Viking [2][25R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Dragon [2][40R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Defense Tower[5R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Boss[100R]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "----------Stats-------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+100);
                std::map<uint64, PlayerData>::iterator itr = PlayerDataContainer.find(player->GetGUID());
                if (itr != PlayerDataContainer.end())
                {
                    char msg [25];
                    snprintf(msg, 25, "Resources: %u \n Score: %u", itr->second.resources, itr->second.score);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, msg, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+101);
                }
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
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Nevermind..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+99);
        player->SEND_GOSSIP_MENU(1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions) OVERRIDE
    {
        player->PlayerTalkClass->ClearMenus();
        npc_player_twoAI* gameAI = CAST_AI(npc_player_twoAI, creature->GetAI());

        if (gameAI->Queue.size() == 3) // Don't want too many out there
        {
            player->GetSession()->SendNotification("Exceeded queue size!");
            return false;
        }

        if (actions == GOSSIP_ACTION_INFO_DEF+1)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 10)
            {
                gameAI->Queue[NPC_VIKING] = 3;
                PlayerDataContainer[player->GetGUID()].resources -= 10;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+2)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 25)
            {
                gameAI->Queue[NPC_ARMORED_VIKING] = 2;
                PlayerDataContainer[player->GetGUID()].resources -= 25;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+3)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 40)
            {
                gameAI->Queue[NPC_DRAGON] = 2;
                PlayerDataContainer[player->GetGUID()].resources -= 40;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+4)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 5)
            {
                gameAI->Queue[NPC_TOTEM_TOWER] = 1;
                PlayerDataContainer[player->GetGUID()].resources -= 5;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+5)
        {
            if (PlayerDataContainer[player->GetGUID()].resources >= 100)
            {
                gameAI->Queue[NPC_PLAYER_TWO_BOSS] = 1;
                PlayerDataContainer[player->GetGUID()].resources -= 100;
            }
            else
                player->GetSession()->SendAreaTriggerMessage("Not enough resources!");
        }
        else if (actions == GOSSIP_ACTION_INFO_DEF+99)
            player->CLOSE_GOSSIP_MENU();
        player->CLOSE_GOSSIP_MENU();
        return true;
	}

	struct npc_player_twoAI : public ScriptedAI
	{
        npc_player_twoAI(Creature* creature) : ScriptedAI(creature), summons(me) { }

        uint32 gameTimer;
        uint32 lightningBoltTimer;
        std::map<uint32, uint16> Queue; // [Entry] - [Size]

        void Reset() OVERRIDE
        {
            gameTimer = urand(6000, 13000);
            lightningBoltTimer = 4000;
            summons.DespawnAll();
        }

        void JustDied(Unit* /*killer*/) OVERRIDE
        {
            summons.DespawnAll();
            eventOver = true;
            me->setFaction(35);
            PlayerDataContainer.clear();
            taken = 0;
        }

        void JustSummon(Creature* summoned)
        {
            summons.Summon(summoned);
        }

        void DamageTaken(Unit* attacker, uint32 &damage) OVERRIDE
        {
            if (attacker->GetTypeId() == TYPEID_PLAYER)
            {
                ChatHandler(attacker->ToPlayer()->GetSession()).SendSysMessage("You cannot attack Player Two's minion!");
                DoCast(me, SPELL_RENEW, false);
                attacker->ToPlayer()->TeleportTo(0, creatureSpawns[6].GetPositionX(), creatureSpawns[6].GetPositionY() + 5, creatureSpawns[6].GetPositionZ(), creatureSpawns[6].GetOrientation());
            }
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (gameTimer <= diff)
            {
                if (me->IsAlive() && eventOver)
                    summons.DespawnAll();
                if (PlayerDataContainer.size() == 2)
                    SpawnCreatureInQueue(); // Will spawn a random creature in the queue
                gameTimer = urand(6000, 13000);
            }
            else
                gameTimer -= diff;
            if (!UpdateVictim())
                return;

            if (lightningBoltTimer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_LIGHTNING_BOLT, true);
                lightningBoltTimer = 4000;
            }
            else
                lightningBoltTimer -= diff;
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

                    me->SummonCreature(itr->first, creatureSpawns[5].GetPositionX(), creatureSpawns[5].GetPositionY(), creatureSpawns[5].GetPositionZ(), creatureSpawns[5].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN)->setFaction(me->getFaction());
                    UpdateCreatureQueueSize(itr->first);
                    i++;
                }
            }
        }
    private:
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const OVERRIDE
    {
        return new npc_player_twoAI(creature);
    }
};

class npc_player_two_units : public CreatureScript
{
public:
	npc_player_two_units() : CreatureScript("npc_player_two_units") { }

    struct npc_player_two_unitsAI : public ScriptedAI
    {
        npc_player_two_unitsAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() OVERRIDE
        {
        }

        void JustDied(Unit* /*killer*/) OVERRIDE
        {
            IncrementStats(urand(5, 20), urand(5, 15), 1);
        }

        void DamageTaken(Unit* attacker, uint32 &damage) OVERRIDE
        {
            if (attacker->GetTypeId() == TYPEID_PLAYER)
            {
                ChatHandler(attacker->ToPlayer()->GetSession()).SendSysMessage("You cannot attack Player Two's minion!");
                DoCast(me, SPELL_RENEW, false);
                me->AI()->Reset();
                me->AttackStop();
                me->SetTarget(0);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, creatureSpawns[0].GetPositionX(), creatureSpawns[0].GetPositionY() - 5, creatureSpawns[0].GetPositionZ());
                attacker->ToPlayer()->TeleportTo(0, creatureSpawns[6].GetPositionX(), creatureSpawns[6].GetPositionY(), creatureSpawns[6].GetPositionZ(), creatureSpawns[6].GetOrientation());
            }
        }

        void UpdateAI(uint32 diff) OVERRIDE
        {
            if (!UpdateVictim())
                me->GetMotionMaster()->MovePoint(0, creatureSpawns[0].GetPositionX(), creatureSpawns[0].GetPositionY(), creatureSpawns[0].GetPositionZ());
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const OVERRIDE
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