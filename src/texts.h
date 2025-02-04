enum Localization
{
  LOCALIZATION_ENG,
  LOCALIZATION_GER,

  LOCALIZATION_COUNT
};

enum StringID
{
  STRING_GAME_TITLE,
  STRING_MADE_IN_CPP,

  STRING_COUNT
};

const char* Strings[(int)STRING_COUNT * (int)LOCALIZATION_COUNT];
static Localization localization = LOCALIZATION_ENG;

void init_strings()
{
  // @TODO TITLE_FIX_ISSUE Find better way to center this based on string length
  Strings[(int)LOCALIZATION_ENG + (int)STRING_GAME_TITLE] = " Fields of Oblivion";
  Strings[(int)LOCALIZATION_ENG + (int)STRING_MADE_IN_CPP] = "Made in C++";

  // German Translation, ö, Ö, ä, Ä, don't work like this!!!! I myself have no solution YET!
  // But we will have a solution in the future
  Strings[(int)LOCALIZATION_GER * STRING_COUNT + (int)STRING_GAME_TITLE] = "";
  Strings[(int)LOCALIZATION_GER * STRING_COUNT + (int)STRING_MADE_IN_CPP] = "Geschrieben in C++";
}

// This is the get string function

const char* _(StringID stringID)
{
  return Strings[(int)localization * (int)STRING_COUNT + (int)stringID];
}