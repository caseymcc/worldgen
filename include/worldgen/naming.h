#ifndef _worldgen_naming_h_
#define _worldgen_naming_h_

#include "worldgen/export.h"
#include "worldgen/settlement.h"

#include <string>
#include <vector>

namespace worldgen
{

//Nothing in this world could be talked about. A place was a pair of coordinates and a realm was
//"Elf realm", which is fine for a debug view and useless the moment anything shows a neighbour to
//somebody.
//
//Names are built rather than drawn from a list, for two reasons. A list runs out, and worse, a list
//has no accent: every people would name their towns from the same bag and a dwarven hold would
//sound like an elven one. Each people gets its own sounds instead, so its places sound related to
//each other and unlike anybody else's.
//
//And where the ground says something, the name says it too. Real place names are mostly descriptions
//that stopped being read as descriptions - a ford, a hill, a river mouth, a crossing - which is why
//a made-up name that means nothing reads as made up.
struct Phonology
{
    std::vector<std::string> m_onsets;   //what a syllable may start with
    std::vector<std::string> m_vowels;
    std::vector<std::string> m_codas;    //and end with
    std::vector<std::string> m_endings;  //what the people's names tend to finish on

    //what this people calls the things it names places after
    std::string m_ford;
    std::string m_bridge;
    std::string m_hill;
    std::string m_river;
    std::string m_mouth;
    std::string m_wood;
    std::string m_stone;
    std::string m_mine;
    std::string m_coast;
    std::string m_field;

    bool m_joined;   //whether the describing word is stuck on the end or stands as a second word
};

//A hash, so a name is a function of what it names rather than of the order things were generated in.
//Insert a settlement earlier in the list and every later name would otherwise change.
inline uint32_t nameHash(uint32_t a, uint32_t b)
{
    uint32_t hash=a*0x9E3779B1u;

    hash^=b*0x85EBCA77u;
    hash^=hash>>15;
    hash*=0xC2B2AE3Du;
    hash^=hash>>13;
    hash*=0x27D4EB2Fu;
    hash^=hash>>16;

    return hash;
}

inline const std::string &pick(const std::vector<std::string> &from, uint32_t roll)
{
    static const std::string empty;

    if(from.empty())
        return empty;

    return from[roll%from.size()];
}

inline std::string capitalise(const std::string &word)
{
    std::string out=word;

    if((!out.empty())&&(out[0]>='a')&&(out[0]<='z'))
        out[0]=(char)(out[0]-'a'+'A');

    return out;
}

//A stem of one or two syllables in this people's sounds.
inline std::string nameStem(const Phonology &sounds, uint32_t seed)
{
    uint32_t roll=nameHash(seed, 0x51ed2701u);
    std::string stem;
    int syllables=1+(int)((roll>>3)%2);

    for(int i=0; i<syllables; ++i)
    {
        uint32_t part=nameHash(seed, 0x1000u+(uint32_t)i);

        stem+=pick(sounds.m_onsets, part);
        stem+=pick(sounds.m_vowels, part>>7);

        //a coda between syllables would make most of these unpronounceable
        if((i==(syllables-1))||(((part>>13)&3)==0))
            stem+=pick(sounds.m_codas, part>>17);
    }

    return stem;
}

//What this place is, in one word, if the ground says anything worth saying. Empty when it does not,
//and then the name is just the stem and its people's usual ending.
inline std::string nameElement(const Phonology &sounds, const InfluenceCell &cell, bool crossing)
{
    if(crossing)
        return (cell.crossing==(int)CrossingKind::Bridge)?sounds.m_bridge:sounds.m_ford;

    if((cell.coast!=0)&&(cell.water==WaterBody::River))
        return sounds.m_mouth;

    if(cell.water==WaterBody::River)
        return sounds.m_river;

    if(cell.coast!=0)
        return sounds.m_coast;

    if((cell.deposit!=DepositType::None)&&cell.depositExposed)
        return sounds.m_mine;

    if(cell.forest)
        return sounds.m_wood;

    return std::string();
}

//The name of a place, from its people's sounds and from what the ground there is.
inline std::string placeName(const Phonology &sounds, const InfluenceCell &cell, uint32_t seed,
    bool crossing=false)
{
    std::string stem=nameStem(sounds, seed);
    std::string element;

    //Only sometimes from the ground. Naming every place after what is under it sounds systematic
    //rather than lived-in: a country of forest came out as Dimwood, Leanwood, Mudbroukwood and
    //Wedekwood, which is one joke told four times. A crossing always earns its name - those are the
    //ones real places are actually named for - and the rest is mostly the people's usual endings.
    if(crossing||((nameHash(seed, 0x3c6ef372u)%100)<35))
        element=nameElement(sounds, cell, crossing);

    if(element.empty())
        element=pick(sounds.m_endings, nameHash(seed, 0x7f4a7c15u));

    if(sounds.m_joined)
        return capitalise(stem+element);

    return capitalise(stem)+" "+capitalise(element);
}

//"of the Elfs" is nobody's realm. Enough of a plural to cover the peoples anyone is likely to
//write down, and a bare "s" for the rest.
inline std::string pluralOf(const std::string &people)
{
    size_t length=people.size();

    if(length>=1)
    {
        char last=people[length-1];

        if((last=='f')||(last=='F'))
            return people.substr(0, length-1)+"ves";

        if((last=='s')||(last=='x')||(last=='z'))
            return people+"es";

        if((length>=2)&&(people[length-2]=='f')&&((last=='e')||(last=='E')))
            return people.substr(0, length-2)+"ves";
    }

    return people+"s";
}

//A realm is named for its people and its seat, the way most of them are.
inline std::string realmName(const Phonology &sounds, const std::string &people, uint32_t seed)
{
    std::string stem=capitalise(nameStem(sounds, seed));
    std::string folk=pluralOf(people);

    switch(nameHash(seed, 0x2545f491u)%3)
    {
    case 0: return stem;
    case 1: return stem+" of the "+folk;
    default: break;
    }

    return "The "+folk+" of "+stem;
}

//--- how each people sounds ---
//Chosen to be told apart when read aloud rather than to be authentic to anything: soft and long for
//the elves, short and hard for the dwarves, blunt for the orcs, homely for the halflings.

inline Phonology humanPhonology()
{
    Phonology sounds;

    sounds.m_onsets={"b", "br", "d", "f", "g", "h", "k", "l", "m", "n", "r", "s", "st", "t", "w"};
    sounds.m_vowels={"a", "e", "i", "o", "u", "ea", "ou"};
    sounds.m_codas={"n", "r", "l", "m", "d", "k", "st", "rd"};
    sounds.m_endings={"ton", "ham", "by", "wick", "field", "stead"};
    sounds.m_ford="ford";
    sounds.m_bridge="bridge";
    sounds.m_hill="hill";
    sounds.m_river="water";
    sounds.m_mouth="mouth";
    sounds.m_wood="wood";
    sounds.m_stone="stone";
    sounds.m_mine="delve";
    sounds.m_coast="haven";
    sounds.m_field="field";
    sounds.m_joined=true;

    return sounds;
}

inline Phonology elfPhonology()
{
    Phonology sounds;

    sounds.m_onsets={"ael", "cel", "el", "fin", "gal", "il", "lir", "mel", "nal", "sil", "thal", "van"};
    sounds.m_vowels={"a", "e", "i", "ae", "ia", "io"};
    sounds.m_codas={"l", "n", "r", "th", "s"};
    sounds.m_endings={"iel", "aria", "eth", "wen", "dor", "las"};
    sounds.m_ford="athren";
    sounds.m_bridge="andel";
    sounds.m_hill="orod";
    sounds.m_river="duin";
    sounds.m_mouth="ethir";
    sounds.m_wood="lorn";
    sounds.m_stone="gond";
    sounds.m_mine="nogrod";
    sounds.m_coast="falas";
    sounds.m_field="loth";
    sounds.m_joined=false;

    return sounds;
}

inline Phonology dwarfPhonology()
{
    Phonology sounds;

    sounds.m_onsets={"b", "br", "d", "dr", "g", "gr", "k", "kh", "m", "n", "th", "tr", "v"};
    sounds.m_vowels={"a", "o", "u", "i"};
    sounds.m_codas={"k", "r", "n", "rn", "zd", "m", "g", "th"};
    sounds.m_endings={"heim", "dun", "hold", "delf", "grim"};
    sounds.m_ford="wade";
    sounds.m_bridge="span";
    sounds.m_hill="tor";
    sounds.m_river="run";
    sounds.m_mouth="gate";
    sounds.m_wood="holt";
    sounds.m_stone="stone";
    sounds.m_mine="delve";
    sounds.m_coast="strand";
    sounds.m_field="acre";
    sounds.m_joined=true;

    return sounds;
}

inline Phonology orcPhonology()
{
    Phonology sounds;

    sounds.m_onsets={"g", "gr", "k", "kr", "m", "n", "sh", "sk", "th", "ur", "z"};
    sounds.m_vowels={"a", "u", "o"};
    sounds.m_codas={"g", "k", "z", "sh", "rk", "gg", "n"};
    sounds.m_endings={"nak", "grod", "uk", "mag", "thar"};
    sounds.m_ford="wash";
    sounds.m_bridge="span";
    sounds.m_hill="crag";
    sounds.m_river="flow";
    sounds.m_mouth="maw";
    sounds.m_wood="scrag";
    sounds.m_stone="rock";
    sounds.m_mine="pit";
    sounds.m_coast="shore";
    sounds.m_field="graze";
    sounds.m_joined=true;

    return sounds;
}

inline Phonology halflingPhonology()
{
    Phonology sounds;

    sounds.m_onsets={"b", "br", "d", "f", "h", "l", "m", "n", "p", "t", "w"};
    sounds.m_vowels={"a", "e", "i", "o", "u", "oo"};
    sounds.m_codas={"n", "l", "m", "d", "t", "ck"};
    sounds.m_endings={"bury", "bottom", "burrow", "hollow", "meadow", "barrow"};
    sounds.m_ford="ford";
    sounds.m_bridge="bridge";
    sounds.m_hill="down";
    sounds.m_river="brook";
    sounds.m_mouth="reach";
    sounds.m_wood="copse";
    sounds.m_stone="stone";
    sounds.m_mine="dig";
    sounds.m_coast="strand";
    sounds.m_field="field";
    sounds.m_joined=true;

    return sounds;
}

//Matched to the default species list by position, and falling back to the human sounds so a caller
//that supplies its own peoples still gets names rather than nothing.
inline Phonology phonologyFor(size_t species)
{
    switch(species)
    {
    case 1: return elfPhonology();
    case 2: return dwarfPhonology();
    case 3: return orcPhonology();
    case 4: return halflingPhonology();
    default: break;
    }

    return humanPhonology();
}

}//namespace worldgen

#endif //_worldgen_naming_h_
