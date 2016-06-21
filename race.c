/**************************************************************************
*  File: race.c                                               LuminariMUD *
*  Usage: Source file for race-specific code.                             *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
**************************************************************************/

/** Help buffer the global variable definitions */
#define __RACE_C__

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"
#include "act.h"
#include "handler.h"
#include "comm.h"
#include "race.h"
#include "feats.h"

/* defines */
#define Y   TRUE
#define N   FALSE
/* racial classification for PC races */
#define IS_NORMAL  0
#define IS_ADVANCE 1
#define IS_EPIC_R  2

/* some pre setup here */
struct race_data race_list[NUM_EXTENDED_RACES];

/* Zusuk, 02/2016:  Start notes here!
 * RACE_ these are specific race defines, eventually should be a massive list
 *       of every race in our world (ex: iron golem)
 * SUBRACE_ these are subraces for NPC's, currently set to maximum 3, some
 *          mechanics such as resistances are built into these (ex: fire, goblinoid)
 * PC_SUBRACE_ these are subraces for PC's, only used for animal shapes spell
 *             currently, use to be part of the wildshape system (need to phase this out)
 * RACE_TYPE_ this is like the family the race belongs to, like an iron golem
 *            would be RACE_TYPE_CONSTRUCT
 */

/* start race code! */

/* this will set the appropriate gender for a given race */
void set_race_genders(int race, int neuter, int male, int female) {
  race_list[race].genders[0] = neuter;
  race_list[race].genders[1] = male;
  race_list[race].genders[2] = female;
}

/* this will set the ability modifiers of the given race to whatever base
   stats are, to be used for both PC and wildshape forms */
const char *abil_mod_names[NUM_ABILITY_MODS + 1] = {
  /* an unfortunate necessity to make this constant array - we didn't make
     the modifiers same order as the structs.h version */
  "Strength",
  "Constitution",
  "Intelligence",
  "Wisdom",
  "Dexterity",
  "Charisma",
  "\n"
};
void set_race_abilities(int race, int str_mod, int con_mod, int int_mod,
        int wis_mod, int dex_mod, int cha_mod) {
  race_list[race].ability_mods[0] = str_mod;
  race_list[race].ability_mods[1] = con_mod;
  race_list[race].ability_mods[2] = int_mod;
  race_list[race].ability_mods[3] = wis_mod;
  race_list[race].ability_mods[4] = dex_mod;
  race_list[race].ability_mods[5] = cha_mod;
}

/* appropriate alignments for given race */
void set_race_alignments(int race, int lg, int ng, int cg, int ln, int tn, int cn,
        int le, int ne, int ce) {
  race_list[race].alignments[0] = lg;
  race_list[race].alignments[1] = ng;
  race_list[race].alignments[2] = cg;
  race_list[race].alignments[3] = ln;
  race_list[race].alignments[4] = tn;
  race_list[race].alignments[5] = cn;
  race_list[race].alignments[6] = le;
  race_list[race].alignments[7] = ne;
  race_list[race].alignments[8] = ce;
}

/* set the attack types this race will use when not wielding */
void set_race_attack_types(int race, int hit, int sting, int whip, int slash,
        int bite, int bludgeon, int crush, int pound, int claw, int maul,
        int thrash, int pierce, int blast, int punch, int stab, int slice,
        int thrust, int hack, int rake, int peck, int smash, int trample,
        int charge, int gore) {
  race_list[race].attack_types[hit] = hit;
  race_list[race].attack_types[sting] = sting;
  race_list[race].attack_types[whip] = whip;
  race_list[race].attack_types[slash] = slash;
  race_list[race].attack_types[bite] = bite;
  race_list[race].attack_types[bludgeon] = bludgeon;
  race_list[race].attack_types[crush] = crush;
  race_list[race].attack_types[pound] = pound;
  race_list[race].attack_types[claw] = claw;
  race_list[race].attack_types[maul] = maul;
  race_list[race].attack_types[thrash] = thrash;
  race_list[race].attack_types[pierce] = pierce;
  race_list[race].attack_types[blast] = blast;
  race_list[race].attack_types[punch] = punch;
  race_list[race].attack_types[stab] = stab;
  race_list[race].attack_types[slice] = slice;
  race_list[race].attack_types[thrust] = thrust;
  race_list[race].attack_types[hack] = hack;
  race_list[race].attack_types[rake] = rake;
  race_list[race].attack_types[peck] = peck;
  race_list[race].attack_types[smash] = smash;
  race_list[race].attack_types[trample] = trample;
  race_list[race].attack_types[charge] = charge;
  race_list[race].attack_types[gore] = gore;
}

/* function to initialize the whole race list to empty values */
void initialize_races(void) {
  int i = 0;

  for (i = 0; i < NUM_EXTENDED_RACES; i++) {
    /* displaying the race */
    race_list[i].name = NULL;
    race_list[i].type = NULL;
    race_list[i].type_color = NULL;
    race_list[i].abbrev = NULL;
    race_list[i].abbrev_color = NULL;
    
    /* displaying more race details (extension) */
    race_list[i].descrip = NULL;
    race_list[i].morph_to_char = NULL;
    race_list[i].morph_to_room = NULL;
    
    /* the rest of the values */
    race_list[i].family = RACE_TYPE_UNDEFINED;
    race_list[i].size = SIZE_MEDIUM;
    race_list[i].is_pc = FALSE;
    race_list[i].level_adjustment = 0;
    race_list[i].unlock_cost = 99999;
    race_list[i].epic_adv = IS_NORMAL;
        
    /* handle outside add_race() */
    set_race_genders(i, N, N, N);
    set_race_abilities(i, 0, 0, 0, 0, 0, 0);
    set_race_alignments(i, N, N, N, N, N, N, N, N, N);
    set_race_attack_types(i, Y, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N,
                             N, N, N, N, N, N, N);
    
    /* any linked lists to initailze? */
  }
}

/* papa assign-function for adding races to the race list */
void add_race(int race,
        char *name, char *type, char *type_color, char *abbrev, char *abbrev_color,
        ubyte family, byte size, sbyte is_pc, ubyte level_adjustment, int unlock_cost,
        byte epic_adv) {
  
  /* displaying the race */
  race_list[race].name = strdup(name); /* lower case no-space */
  race_list[race].type = strdup(type); /* capitalized space, no color */
  race_list[race].type_color = strdup(type_color); /* capitalized space, color */
  race_list[race].abbrev = strdup(abbrev); /* 4 letter abbrev, no color */
  race_list[race].abbrev_color = strdup(abbrev_color); /* 4 letter abbrev, color */
  
  /* assigning values */
  race_list[race].family = family;
  race_list[race].size = size;
  race_list[race].is_pc = is_pc;
  race_list[race].level_adjustment = level_adjustment;
  race_list[race].unlock_cost = unlock_cost;
  race_list[race].epic_adv = epic_adv;  
}

/* extension of details added to race */
void set_race_details(int race,
        char *descrip, char *morph_to_char, char *morph_to_room) {
  
  race_list[race].descrip = strdup(descrip); /* Description of race */  
  /* message to send to room if transforming to this particular race */
  race_list[race].morph_to_char = strdup(morph_to_char);
  race_list[race].morph_to_room = strdup(morph_to_room);
}

/*
// fun idea based on favored class system, not currently utilized in our game
void favored_class_female(int race, int favored_class) {
  race_list[race].favored_class[2] = favored_class;
}
*/

/* our little mini struct series for assigning feats to a race  */
/* create/allocate memory for the racefeatassign struct */
struct race_feat_assign* create_feat_assign_races(int feat_num, int level_received,
        bool stacks) {
  struct race_feat_assign *feat_assign = NULL;

  CREATE(feat_assign, struct race_feat_assign, 1);
  feat_assign->feat_num = feat_num;
  feat_assign->level_received = level_received;
  feat_assign->stacks = stacks;

  return feat_assign;
}
/* actual function called to perform the feat assignment */
void feat_race_assignment(int race_num, int feat_num, int level_received,
        bool stacks) {
  struct race_feat_assign *feat_assign = NULL;

  feat_assign = create_feat_assign_races(feat_num, level_received, stacks);

  /*   Link it up. */
  feat_assign->next = race_list[race_num].featassign_list;
  race_list[race_num].featassign_list = feat_assign;
}

/* our little mini struct series for assigning affects to a race  */
/* create/allocate memory for the struct */
struct affect_assign* create_affect_assign(int affect_num, int level_received) {
  struct affect_assign *aff_assign = NULL;

  CREATE(aff_assign, struct affect_assign, 1);
  aff_assign->affect_num = affect_num;
  aff_assign->level_received = level_received;

  return aff_assign;
}
/* actual function called to perform the affect assignment */
void affect_assignment(int race_num, int affect_num, int level_received) {
  struct affect_assign *aff_assign = NULL;

  aff_assign = create_affect_assign(affect_num, level_received);

  /*   Link it up. */
  aff_assign->next = race_list[race_num].affassign_list;
  race_list[race_num].affassign_list = aff_assign;
}

/* determines if ch qualifies for a race */
bool race_is_available(struct char_data *ch, int race_num) {
  /* unfinished ! */
  return FALSE;
}

/*****************************/
/*****************************/

/* this will be a general list of all pc races */
void display_pc_races(struct char_data *ch) {
  struct descriptor_data *d = ch->desc;
  int counter, columns = 0;

  write_to_output(d, "\r\n");
  
  for (counter = 0; counter < NUM_EXTENDED_RACES; counter++) {
    if (race_list[counter].is_pc) {    
      write_to_output(d, "%s%-20.20s %s",
            race_is_available(ch, counter) ? " " : "*",
            race_list[counter].type, 
            !(++columns % 3) ? "\r\n" : "");
    }
  }
  
  write_to_output(d, "\r\n");
  write_to_output(d, "* - not unlocked 'race prereqs <race name>' for details\r\n");
  write_to_output(d, "\r\n");
}

/* display a specific races details */
bool display_race_info(struct char_data *ch, char *racename) {
  int race = -1, stat_mod = 0;
  char buf[MAX_STRING_LENGTH] = { '\0' };  
  static int line_length = 80, i = 0;
  size_t len = 0;
  bool found = FALSE;

  skip_spaces(&racename);
  race = parse_race_long(racename);

  if (race == -1 || race >= NUM_EXTENDED_RACES) {
    /* Not found - Maybe put in a soundex list here? */
    return FALSE;
  }

  /* We found the race, and the race number is stored in 'race'. */
  /* Display the race info, formatted. */
  send_to_char(ch, "\tC\r\n");
  draw_line(ch, line_length, '-', '-');
  
  send_to_char(ch, "\tcRace Name       : \tn%s\r\n", race_list[race].type_color);
  send_to_char(ch, "\tcNormal/Adv/Epic?: \tn%s\r\n", (race_list[race].epic_adv == IS_EPIC_R) ?
    "Epic Race" : (race_list[race].epic_adv == IS_ADVANCE) ? "Advance Race" : "Normal Race");
  send_to_char(ch, "\tcUnlock Cost     : \tn%d Account XP\r\n", race_list[race].unlock_cost);  
  send_to_char(ch, "\tcPlayable Race?  : \tn%s\r\n", race_list[race].is_pc ?
                   "\tnYes\tn" : "\trNo, ask staff\tn");
  
  send_to_char(ch, "\tC");
  draw_line(ch, line_length, '-', '-');
  
  /* build buffer for ability modifiers */
  for (i = 0; i < NUM_ABILITY_MODS; i++) {
    stat_mod = race_list[race].ability_mods[i];
    if (stat_mod) {
      found = TRUE;
      len += snprintf(buf + len, sizeof (buf) - len,
          "%s %s%d | ",
          abil_mod_names[i], (stat_mod > 0) ? "+" : "", stat_mod);
      
    }
  }
  
  send_to_char(ch, "\tcRace Size       : \tn%s\r\n", sizes[race_list[race].size]);
  send_to_char(ch, "\tcAbility Modifier: \tn%s\r\n", found ? buf : "None");  
      
  send_to_char(ch, "\tC");
  draw_line(ch, line_length, '-', '-');
  
  /* This we will need to buffer and wrap so that it will fit in the space provided. */
  sprintf(buf, "\tcDescription : \tn%s\r\n", race_list[race].descrip);
  send_to_char(ch, strfrmt(buf, line_length, 1, FALSE, FALSE, FALSE));
  
  send_to_char(ch, "\tC");
  draw_line(ch, line_length, '-', '-');
    
  send_to_char(ch, "\tYType: \tRrace feats %s\tY for this race's feat info.\tn\r\n",
    race_list[race].type_color);
  
  send_to_char(ch, "\tC");
  draw_line(ch, line_length, '-', '-');
  send_to_char(ch, "\tn");

  return TRUE;  
}

/* function to view a list of feats race is granted */
bool view_race_feats(struct char_data *ch, char *racename) {
  int race = RACE_UNDEFINED;
  struct race_feat_assign *feat_assign = NULL;
  
  skip_spaces(&racename);
  race = parse_race_long(racename);

  if (race == RACE_UNDEFINED) {
    return FALSE;
  }

  /* level feats */
  if (race_list[race].featassign_list != NULL) {
    /*  This race has feat assignment! Traverse the list and list. */
    for (feat_assign = race_list[race].featassign_list; feat_assign != NULL;
            feat_assign = feat_assign->next) {
      if (feat_assign->level_received > 0) /* -1 is just race feat assign */
        send_to_char(ch, "Level: %-2d, Stacks: %-3s, Feat: %s\r\n",
                   feat_assign->level_received,
                   feat_assign->stacks ? "Yes" : "No",
                   feat_list[feat_assign->feat_num].name);
    }
  }
  send_to_char(ch, "\r\n");
  
  return TRUE;
}

/**************************************/

/* entry point for race command - getting race info */
ACMD(do_race) {
  char arg[80];
  char arg2[80];
  char *racename;

  /*  Have to process arguments like this
   *  because of the syntax - race info <racename> */
  racename = one_argument(argument, arg);
  one_argument(racename, arg2);

  /* no argument, or general list of classes */
  if (is_abbrev(arg, "list") || !*arg) {
    display_pc_races(ch);
    
  /* race info - specific info on given race */    
  } else if (is_abbrev(arg, "info")) {

    if (!strcmp(racename, "")) {
      send_to_char(ch, "\r\nYou must provide the name of a race.\r\n");
    } else if(!display_race_info(ch, racename)) {
      send_to_char(ch, "Could not find that race.\r\n");
    }
    
  /* race feat - list of free feats for given race */    
  } else if (is_abbrev(arg, "feats")) {

    if (!strcmp(racename, "")) {
      send_to_char(ch, "\r\nYou must provide the name of a race.\r\n");
    } else if(!view_race_feats(ch, racename)) {
      send_to_char(ch, "Could not find that race.\r\n");
    }

  }
  
  send_to_char(ch, "\tDUsage: race <list|info|feats> <race name>\tn\r\n");
}

/*****************************/
/*****************************/

/* here is the actual race list */
void assign_races(void) {
  /* initialization */
  initialize_races();

  /* begin listing */

  /************/
  /* Humanoid */
  /************/
  
  /******/
  /* PC */
  /******/
  
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_HUMAN, "human", "Human", "\tBHuman\tn", "Humn", "\tBHumn\tn",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_HUMANOID, SIZE_MEDIUM, TRUE, 0,    0,      IS_NORMAL);
    set_race_details(RACE_HUMAN,
      /*descrip*/"Humans possess exceptional drive and a great capacity to endure "
      "and expand, and as such are currently the dominant race in the world. Their "
      "empires and nations are vast, sprawling things, and the citizens of these "
      "societies carve names for themselves with the strength of their sword arms "
      "and the power of their spells. Humanity is best characterized by its "
      "tumultuousness and diversity, and human cultures run the gamut from savage "
      "but honorable tribes to decadent, devil-worshiping noble families in the most "
      "cosmopolitan cities. Humans' curiosity and ambition often triumph over their "
      "predilection for a sedentary lifestyle, and many leave their homes to explore "
      "the innumerable forgotten corners of the world or lead mighty armies to conquer "
      "their neighbors, simply because they can.",
      /*morph to-char*/"Your body twists and contorts painfully until your form becomes Human.",
      /*morph to-room*/"$n's body twists and contorts painfully until $s form becomes Human.");
    set_race_genders(RACE_HUMAN, N, Y, Y); /* n m f */
    set_race_abilities(RACE_HUMAN, 0, 0, 0, 0, 0, 0); /* str con int wis dex cha */
    set_race_alignments(RACE_HUMAN, Y, Y, Y, Y, Y, Y, Y, Y, Y); /* law-good -> cha-evil */  
    set_race_attack_types(RACE_HUMAN,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        Y,  N,    N,   N,    N,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    Y,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
    /* feat assignment */
    /*                   race-num    feat                  lvl stack */
    feat_race_assignment(RACE_HUMAN, FEAT_QUICK_TO_MASTER, 1,  N);
    feat_race_assignment(RACE_HUMAN, FEAT_SKILLED,         1,  N);
    /* affect assignment */
    /*                  race-num  affect            lvl */
    /**TEST**/affect_assignment(RACE_HUMAN, AFF_DETECT_ALIGN, 1);
  /****************************************************************************/
            
     /*
  add_race(RACE_DROW_ELF, "drow", "Drow", "\tmDrow\tn", "Drow", "\tmDrow\tn",
           RACE_TYPE_HUMANOID, SIZE_MEDIUM, FALSE, 2);
    //favored_class_female(RACE_DROW_ELF, CLASS_CLERIC);
  add_race(RACE_HALF_ELF, "half elf", "HalfElf", "Half Elf", RACE_TYPE_HUMANOID, N, Y, Y, 0, 0, 0, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_WIZARD, SKILL_LANG_ELVEN, 0);
  add_race(RACE_ELF, "elf", "Elf", "Elf", RACE_TYPE_HUMANOID, N, Y, Y, 0, 0, 0, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_WIZARD, SKILL_LANG_ELVEN, 0);
  add_race(RACE_CRYSTAL_DWARF, "crystal dwarf", "CrystalDwarf", "Crystal Dwarf", RACE_TYPE_HUMANOID, N, Y, Y, 0, 2, 0, 0, 0, -4,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_WARRIOR, SKILL_LANG_UNDERCOMMON, 1);
  add_race(RACE_DWARF, "dwarf", "Dwarf", "Dwarf", RACE_TYPE_HUMANOID, N, Y, Y, 0, 2, 0, 0, 0, -4,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_WARRIOR, SKILL_LANG_DWARVEN, 1);
  add_race(RACE_HALFLING, "halfling", "Halfling", "Halfling", RACE_TYPE_HUMANOID, N, Y, Y, 0, 0, 0, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_SMALL, TRUE, CLASS_UNDEFINED, SKILL_LANG_COMMON, 0);
  add_race(RACE_ROCK_GNOME, "rock gnome", "RkGnome", "Rock Gnome", RACE_TYPE_HUMANOID, N, Y, Y, -2, 0, 2, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_SMALL, FALSE, CLASS_WIZARD, SKILL_LANG_GNOME, 0);
  add_race(RACE_DEEP_GNOME, "svirfneblin", "Svfnbln", "Svirfneblin", RACE_TYPE_HUMANOID, N, Y, Y, -2, 0, 0, 2, 2, -4,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_SMALL, FALSE, CLASS_ROGUE, SKILL_LANG_GNOME, 3);
  add_race(RACE_GNOME, "gnome", "Gnome", "Gnome", RACE_TYPE_HUMANOID, N, Y, Y, 0, 0, 2, -2, 2, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_SMALL, TRUE, CLASS_UNDEFINED, SKILL_LANG_GNOME, 0);
  add_race(RACE_HALF_ORC, "half orc", "HalfOrc", "Half Orc", RACE_TYPE_HUMANOID, N, Y, Y, 2, 0, -2, 0, 0, -2,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_BERSERKER, SKILL_LANG_ORCISH, 0);
  add_race(RACE_ORC, "orc", "Orc", "Orc", RACE_TYPE_HUMANOID, N, Y, Y, 2, 2, -2, -2, 0, -2,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, FALSE, CLASS_BERSERKER, SKILL_LANG_ORCISH, 0);
  */
    
    
  /**********/
  /* Animal */
  /**********/
              
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_EAGLE, "eagle", "Eagle", "Eagle", "Eagl", "Eagl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_SMALL, FALSE, 0,      0,      IS_NORMAL);
    set_race_attack_types(RACE_EAGLE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   Y,   Y,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_DINOSAUR, "dinosaur", "Dinosaur", "Dinosaur", "Dino", "Dino",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_DINOSAUR,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    Y,      N,     N);
  /****************************************************************************/

  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_ELEPHANT, "elephant", "Elephant", "Elephant", "Elep", "Elep",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_ELEPHANT,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    Y,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_LEOPARD, "leopard", "Leopard", "Leopard", "Leop", "Leop",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LEOPARD,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_LION, "lion", "Lion",       "Lion",     "Lion", "Lion",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LION,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*            simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_TIGER, "tiger", "Tiger",   "Tiger",     "Tigr", "Tigr",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_TIGER,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_BLACK_BEAR, "black bear", "Black Bear", "Black Bear", "BlBr", "BlBr",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_BLACK_BEAR,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_BROWN_BEAR, "brown bear", "Brown Bear", "Brown Bear", "BrBr", "BrBr",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_BROWN_BEAR,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_POLAR_BEAR, "polar bear", "Polar Bear", "Polar Bear", "PlBr", "PlBr",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_POLAR_BEAR,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/

  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_RHINOCEROS, "rhinoceros", "Rhinoceros", "Rhinoceros", "Rino", "Rino",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_RHINOCEROS,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      Y,     Y);
  /****************************************************************************/
  
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_BOAR, "boar", "Boar", "Boar", "Boar", "Boar",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_BOAR,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      Y,     Y);
  /****************************************************************************/
        
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_APE, "ape", "Ape", "Ape", "Ape", "Ape",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_APE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        Y,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    Y,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
  
  /****************************************************************************/
  /*                        simple-name, no-color-name, color-name, abbrev, color-abbrev*/
  add_race(RACE_RAT, "rat", "Rat", "Rat", "Rat", "Rat",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_TINY, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_RAT,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_WOLF, "wolf", "Wolf", "Wolf", "Wolf", "Wolf",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_WOLF,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/

    /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HORSE, "horse", "Horse", "Horse", "Hors", "Hors",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HORSE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    Y,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_CONSTRICTOR_SNAKE, "constrictor snake", "Constrictor Snake", "Constrictor Snake", "CSnk", "CSnk",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_CONSTRICTOR_SNAKE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_GIANT_CONSTRICTOR_SNAKE, "giant constrictor snake", "Giant Constrictor Snake",
    "Giant Constrictor Snake", "GCSk", "GCSk",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_GIANT_CONSTRICTOR_SNAKE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MEDIUM_VIPER, "medium viper", "Medium Viper", "Medium Viper", "MVip", "MVip",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MEDIUM_VIPER,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_LARGE_VIPER, "large viper", "Large Viper", "Large Viper", "LVip", "LVip",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LARGE_VIPER,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HUGE_VIPER, "huge viper", "Huge Viper", "Huge Viper", "HVip", "HVip",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HUGE_VIPER,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       Y,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_WOLVERINE, "wolverine", "Wolverine", "Wolverine", "Wlvr", "Wlvr",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_WOLVERINE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_CROCODILE, "crocodile", "Crocodile", "Crocodile", "Croc", "Croc",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_CROCODILE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_GIANT_CROCODILE, "giant crocodile", "Giant Crocodile", "Giant Crocodile", "GCrc", "GCrc",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_GIANT_CROCODILE,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_CHEETAH, "cheetah", "Cheetah", "Cheetah", "Chet", "Chet",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ANIMAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_CHEETAH,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    Y,   N,       N,    N,    Y,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/

  /*********/
  /* Plant */
  /*********/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MANDRAGORA, "mandragora", "Mandragora", "Mandragora", "Mand", "Mand",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_PLANT, SIZE_SMALL, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MANDRAGORA,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MYCANOID, "mycanoid", "Mycanoid", "Mycanoid", "Mycd", "Mycd",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_PLANT, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MYCANOID,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_SHAMBLING_MOUND, "shambling mound", "Shambling Mound", "Shambling Mound", "Shmb", "Shmb",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_PLANT, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_SHAMBLING_MOUND,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
    
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_TREANT, "treant", "Treant", "Treant", "Trnt", "Trnt",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_PLANT, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_TREANT,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/

  /*************/
  /* Elemental */
  /*************/
    
    /* FIRE */
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_SMALL_FIRE_ELEMENTAL, "small fire elemental", "Small Fire Elemental", "Small Fire Elemental", "SFEl", "SFEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_SMALL, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_SMALL_FIRE_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     Y,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MEDIUM_FIRE_ELEMENTAL, "medium fire elemental", "Medium Fire Elemental", "Medium Fire Elemental", "MFEl", "MFEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MEDIUM_FIRE_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     Y,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_LARGE_FIRE_ELEMENTAL, "large fire elemental", "Large Fire Elemental", "Large Fire Elemental", "LFEl", "LFEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LARGE_FIRE_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     Y,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HUGE_FIRE_ELEMENTAL, "huge fire elemental", "Huge Fire Elemental", "Huge Fire Elemental", "HFEl", "HFEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HUGE_FIRE_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     Y,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    /* Earth */
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_SMALL_EARTH_ELEMENTAL, "small earth elemental", "Small Earth Elemental", "Small Earth Elemental", "SEEl", "SEEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_SMALL, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_SMALL_EARTH_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MEDIUM_EARTH_ELEMENTAL, "medium earth elemental", "Medium Earth Elemental", "Medium Earth Elemental", "MEEl", "MEEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MEDIUM_EARTH_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_LARGE_EARTH_ELEMENTAL, "large earth elemental", "Large Earth Elemental", "Large Earth Elemental", "LEEl", "LEEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LARGE_EARTH_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HUGE_EARTH_ELEMENTAL, "huge earth elemental", "Huge Earth Elemental", "Huge Earth Elemental", "HEEl", "HEEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HUGE_EARTH_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    N,   N,    N,   Y,       Y,    Y,    N,   N,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    N,     N,   N,   N,   Y,    N,      N,     N);
  /****************************************************************************/
    /* Air */
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_SMALL_AIR_ELEMENTAL, "small air elemental", "Small Air Elemental", "Small Air Elemental", "SAEl", "SAEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_SMALL, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_SMALL_AIR_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        Y,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MEDIUM_AIR_ELEMENTAL, "medium air elemental", "Medium Air Elemental", "Medium Air Elemental", "MAEl", "MAEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MEDIUM_AIR_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        Y,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_LARGE_AIR_ELEMENTAL, "large air elemental", "Large Air Elemental", "Large Air Elemental", "LAEl", "LAEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LARGE_AIR_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        Y,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HUGE_AIR_ELEMENTAL, "huge air elemental", "Huge Air Elemental", "Huge Air Elemental", "HAEl", "HAEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HUGE_AIR_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  N,    Y,   N,    N,   N,       N,    N,    N,   N,   Y,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        Y,    N,    N,   N,    N,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
    /* Water */
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_SMALL_WATER_ELEMENTAL, "small water elemental", "Small Water Elemental", "Small Water Elemental", "SWEl", "SWEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_SMALL, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_SMALL_WATER_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  Y,    N,   N,    N,   N,       N,    N,    N,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    Y,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_MEDIUM_WATER_ELEMENTAL, "medium water elemental", "Medium Water Elemental", "Medium Water Elemental", "MWEl", "MWEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_MEDIUM, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_MEDIUM_WATER_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  Y,    N,   N,    N,   N,       N,    N,    N,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    Y,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_LARGE_WATER_ELEMENTAL, "large water elemental", "Large Water Elemental", "Large Water Elemental", "LWEl", "LWEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_LARGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_LARGE_WATER_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  Y,    N,   N,    N,   N,       N,    N,    N,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    Y,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/
  /****************************************************************************/
  /*                  simple-name, no-color-name, color-name, abbrev (4), color-abbrev (4) */
  add_race(RACE_HUGE_WATER_ELEMENTAL, "huge water elemental", "Huge Water Elemental", "Huge Water Elemental", "HWEl", "HWEl",
           /* race-family, size-class, Is PC?, Lvl-Adj, Unlock, Epic? */
           RACE_TYPE_ELEMENTAL, SIZE_HUGE, FALSE, 0,       0,      IS_NORMAL);
    set_race_attack_types(RACE_HUGE_WATER_ELEMENTAL,
     /* hit sting whip slash bite bludgeon crush pound claw maul thrash pierce */
        N,  Y,    N,   N,    N,   N,       N,    N,    N,   Y,   N,     N,
     /* blast punch stab slice thrust hack rake peck smash trample charge gore */
        N,    N,    N,   N,    Y,     N,   N,   N,   N,    N,      N,     N);
  /****************************************************************************/

  /* monstrous humanoid */
    /*
  add_race(RACE_HALF_TROLL, "half troll", "HalfTroll", "Half Troll", RACE_TYPE_MONSTROUS_HUMANOID, N, Y, Y, 2, 4, 0, 0, 0, -2,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_LARGE, TRUE, CLASS_WARRIOR, SKILL_LANG_GOBLIN, 0);
  */
    
  /* giant */
    /*
  add_race(RACE_HALF_OGRE, "half ogre", "HlfOgre", "Half Ogre", RACE_TYPE_GIANT, N, Y, Y, 6, 4, -2, 0, 2, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_LARGE, FALSE, CLASS_BERSERKER, SKILL_LANG_GIANT, 2);
    */ 

  /* undead */
    /*
  add_race(RACE_SKELETON, "skeleton", "Skeletn", "Skeleton", RACE_TYPE_UNDEAD, Y, N, N, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_ZOMBIE, "zombie", "Zombie", "Zombie", RACE_TYPE_UNDEAD, N, Y, Y, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_GHOUL, "ghoul", "Ghoul", "Ghoul", RACE_TYPE_UNDEAD, N, Y, Y, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_GHAST, "ghast", "Ghast", "Ghast", RACE_TYPE_UNDEAD, N, Y, Y, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_MUMMY, "mummy", "Mummy", "Mummy", RACE_TYPE_UNDEAD, N, Y, Y, 14, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_MOHRG, "mohrg", "Mohrg", "Mohrg", RACE_TYPE_UNDEAD, N, Y, Y, 11, 0, 0, 0, 9, 0,
          N, N, N, N, Y, N, N, N, N, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
    */
    
  /* ooze */

  /* magical beast */
    /*
  add_race(RACE_BLINK_DOG, "blink dog", "BlnkDog", "Blink Dog", RACE_TYPE_MAGICAL_BEAST, N, Y, Y, 0, 0, 0, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
    */
    
  /* fey */
    /*
  add_race(RACE_PIXIE, "pixie", "Pixie", "Pixie", RACE_TYPE_FEY, N, Y, Y, -4, 0, 4, 0, 4, 4,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_TINY, FALSE, CLASS_DRUID, SKILL_LANG_ELVEN, 3);
    */
    
  /* construct */
    /*
  add_race(RACE_IRON_GOLEM, "iron golem", "IronGolem", "Iron Golem", RACE_TYPE_CONSTRUCT, Y, N, N, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, Y, Y, Y, SIZE_LARGE, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
  add_race(RACE_ARCANA_GOLEM, "arcana golem", "ArcanaGolem", "Arcana Golem", RACE_TYPE_CONSTRUCT, Y, N, N, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, Y, Y, Y, SIZE_MEDIUM, TRUE, CLASS_WIZARD, SKILL_LANG_COMMON, 0);
    */
    
  /* outsiders */
    /*
  add_race(RACE_AEON_THELETOS, "aeon theletos", "AeonThel", "Theletos Aeon", RACE_TYPE_OUTSIDER, N, Y, Y, 0, 0, 0, 0, 0, 0,
          Y, Y, Y, Y, Y, Y, Y, Y, Y, SIZE_MEDIUM, FALSE, CLASS_WARRIOR, SKILL_LANG_COMMON, 0);
    */
    
  /* dragon */
    /*
  add_race(RACE_DRAGON_CLOUD, "dragon cloud", "DrgCloud", "Cloud Dragon", RACE_TYPE_DRAGON, N, Y, Y, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, Y, Y, Y, SIZE_HUGE, FALSE, CLASS_WARRIOR, SKILL_LANG_DRACONIC, 0);
    */
    
  /* aberration */
    /*
  add_race(RACE_TRELUX, "trelux", "Trelux", "Trelux", RACE_TYPE_ABERRATION, N, Y, Y, 0, 0, 0, 0, 0, 0,
          N, N, N, N, Y, N, Y, Y, Y, SIZE_SMALL, TRUE, CLASS_WARRIOR, SKILL_LANG_ABERRATION, 0);
    */
    
  /* end listing */
}

// interpret race for interpreter.c and act.wizard.c etc
// notice, epic races are not manually or in-game settable at this stage
int parse_race(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'a': return RACE_HUMAN;
  case 'b': return RACE_ELF;
  case 'c': return RACE_DWARF;
  case 'd': return RACE_HALF_TROLL;
  case 'f': return RACE_HALFLING;
  case 'g': return RACE_H_ELF;
  case 'h': return RACE_H_ORC;
  case 'i': return RACE_GNOME;
  case 'j': return RACE_ARCANA_GOLEM;
  default:  return RACE_UNDEFINED;
  }
}

/* accept short descrip, return race */
int parse_race_long(char *arg) {
  int l = 0; /* string length */

  for (l = 0; *(arg + l); l++) /* convert to lower case */
    *(arg + l) = LOWER(*(arg + l));

  if (is_abbrev(arg, "human")) return RACE_HUMAN;
  if (is_abbrev(arg, "elf")) return RACE_ELF;
  if (is_abbrev(arg, "dwarf")) return RACE_DWARF;
  if (is_abbrev(arg, "half-troll")) return RACE_HALF_TROLL;
  if (is_abbrev(arg, "halftroll")) return RACE_HALF_TROLL;
  if (is_abbrev(arg, "halfling")) return RACE_HALFLING;
  if (is_abbrev(arg, "halfelf")) return RACE_H_ELF;
  if (is_abbrev(arg, "half-elf")) return RACE_H_ELF;
  if (is_abbrev(arg, "halforc")) return RACE_H_ORC;
  if (is_abbrev(arg, "half-orc")) return RACE_H_ORC;
  if (is_abbrev(arg, "gnome")) return RACE_GNOME;
  if (is_abbrev(arg, "arcanagolem")) return RACE_ARCANA_GOLEM;
  if (is_abbrev(arg, "arcana-golem")) return RACE_ARCANA_GOLEM;
  if (is_abbrev(arg, "trelux")) return RACE_TRELUX;
  if (is_abbrev(arg, "crystaldwarf")) return RACE_CRYSTAL_DWARF;
  if (is_abbrev(arg, "crystal-dwarf")) return RACE_CRYSTAL_DWARF;

  return RACE_UNDEFINED;
}

// returns the proper integer for the race, given a character
bitvector_t find_race_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_race(arg[rpos]));

  return (ret);
}

/* Invalid wear flags */
int invalid_race(struct char_data *ch, struct obj_data *obj) {
  if ((OBJ_FLAGGED(obj, ITEM_ANTI_HUMAN) && IS_HUMAN(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_ELF)   && IS_ELF(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_HALF_TROLL)   && IS_HALF_TROLL(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_HALFLING)   && IS_HALFLING(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_H_ELF)   && IS_H_ELF(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_H_ORC)   && IS_H_ORC(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_GNOME)   && IS_GNOME(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_CRYSTAL_DWARF)   && IS_CRYSTAL_DWARF(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_TRELUX)   && IS_TRELUX(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_ARCANA_GOLEM)   && IS_ARCANA_GOLEM(ch)) ||
      (OBJ_FLAGGED(obj, ITEM_ANTI_DWARF) && IS_DWARF(ch)))
        return 1;
  else
        return 0;
}

/*
int get_size(struct char_data *ch) {
  int racenum;

  if (ch == NULL)
    return SIZE_MEDIUM;

  racenum = GET_RACE(ch);

  if (racenum < 0 || racenum >= NUM_EXTENDED_RACES)
    return SIZE_MEDIUM;

  return (GET_SIZE(ch) = ((affected_by_spell(ch, SPELL_ENLARGE_PERSON) ? 1 : 0) + race_list[racenum].size));
}
 */


/* clear up local defines */
#undef Y
#undef N
#undef IS_NORMAL
#undef IS_ADVANCE
#undef IS_EPIC_R

/*EOF*/
