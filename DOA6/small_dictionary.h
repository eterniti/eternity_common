#ifndef SMALL_DICTIONARY_H
#define SMALL_DICTIONARY_H

#include <vector>
#include <string>

static const std::vector<std::string> small_dictionary =
{
    "armour", "body", "face", "glass", "hakama", "helmet", "line", "section", "a01", "a02", "a03", "a04", "b01", "b02", "b03", "b04",
    "c01", "c02", "c03", "c04", "d01", "d02", "d03", "d04", "e01", "e02", "e03", "e04", "f01", "f02", "f03", "f04", "g01", "g02", "g03",
    "g04", "h01", "h02", "h03", "h04", "blend", "BLEND", "blend02", "blend03", "blend04", "body_z", "other01", "other02", "other03", "other04", "hairitem",
    "pants01", "pants02", "pants03", "pants04", "es", "eye", "eyeglass", "matuge", "shitamatu", "tooth", "ribbon1", "ribbon", "hairtie",
    "button", "beard01", "gun", "cap", "cap02", "cap03", "cap04", "ribbon01", "ribbon02", "ribbon03", "ribbon04", "harigane", "lace",
    "other", "a1", "a2", "a3", "a4", "b1", "b2", "b3", "b4", "c1", "c2", "c3", "c4",
    "a01_01", "a01_02", "a01_03", "a01_04", "a02_01", "a02_02", "a02_03", "a02_04", "a03_01", "a03_02", "a03_03", "a03_04", "a04_01", "a04_02", "a04_03", "a04_04",
    "b01_01", "b01_02", "b01_03", "b01_04", "b02_01", "b02_02", "b02_03", "b02_04", "b03_01", "b03_02", "b03_03", "b03_04", "b04_01", "b04_02", "b04_03", "b04_04",
    "c01_01", "c01_02", "c01_03", "c01_04", "c02_01", "c02_02", "c02_03", "c02_04", "c03_01", "c03_02", "c03_03", "c03_04", "c04_01", "c04_02", "c04_03", "c04_04",
    "d01_01", "d01_02", "d01_03", "d01_04", "d02_01", "d02_02", "d02_03", "d02_04", "d03_01", "d03_02", "d03_03", "d03_04", "d04_01", "d04_02", "d04_03", "d04_04",
    "e01_01", "e01_02", "e01_03", "e01_04", "e02_01", "e02_02", "e02_03", "e02_04", "e03_01", "e03_02", "e03_03", "e03_04", "e04_01", "e04_02", "e04_03", "e04_04",
    "f01_01", "f01_02", "f01_03", "f01_04", "f02_01", "f02_02", "f02_03", "f02_04", "f03_01", "f03_02", "f03_03", "f03_04", "f04_01", "f04_02", "f04_03", "f04_04",
    "g01_01", "g01_02", "g01_03", "g01_04", "g02_01", "g02_02", "g02_03", "g02_04", "g03_01", "g03_02", "g03_03", "g03_04", "g04_01", "g04_02", "g04_03", "g04_04",
    "h01_01", "h01_02", "h01_03", "h01_04", "h02_01", "h02_02", "h02_03", "h02_04", "h03_01", "h03_02", "h03_03", "h03_04", "h04_01", "h04_02", "h04_03", "h04_04",
    "pants", "band", "hachimaki", "weapon", "hair4",  "gem", "blueribbon", "blueribbon2", "blueribbon3", "grove1", "grove2", "grove3", "grove4", "himo",
    "katana","d01z", "glove01", "glove02", "glove03", "glove04", "facetears", "tearspool", "material56", "decal", "cap01", "cap1", "cap2", "cap3",
    "hairpin2", "hairpin", "hair01", "hair02", "hair03", "hair04", "fur01", "fur02", "fur03", "fur04", "fusa", "kanzashi", "ornament1", "ornament2",  "ornament3",
    "etc", "etc01", "etc02", "etc03", "line", "line01", "line02", "line03", "mahura", "pbody", "pbody02", "pbody03", "mant", "mant01", "mant02", "mant03", "mant04",
    "mantura", "mantura01", "mantura02", "mantura03", "mantura04", "armour_b1", "armour_c1", "clotha", "clothb", "clothc", "muna", "hip", "hair", "mantua",
    "z", "mob0body", "mob0face", "mob0tooth", "simpl", "wbody", "wfaces", "wtooth", "mobc11", "mobc2", "01", "mob0cloth", "mob0head", "mobchi02b", "kasaw",
    "mobbody", "wcloth", "wface", "02", "03", "04", "wd", "ir", "butterfly", "hana", "metal", "bam", "paper", "bouquet", "bouquet01", "bouquet_01", "lether", "bodyboy",
    "wearboy", "mobcld01c", "mobcld01d", "mobemy", "gag", "hyoutan", "bin", "label", "hone", "kasa", "jinran", "ladder", "mic", "milkbottle", "chain", "emi",
    "grip", "nyowing", "d1", "plastic", "ryuken", "submachinegun", "cup", "utiwa01", "utiwa02", "utiwa03", "fan", "fan02", "fan03", "sensu", "sensu02", "sensu03",
    "fan01", "mascot", "scroll", "mask01", "mask02", "mask03", "file", "phone", "syuriken", "gentian", "flower", "flower1", "flower2", "flower3", "wood", "pen",
    "grass", "container", "pc", "broom", "bucket", "material2", "towl", "rk", "fb", "bone", "zclotha", "liquied", "hotaru", "rkunai", "ryuken1", "pigeon", "a",
    "cos001a", "cos001b", "cos002a", "cos002b", "cos003a", "cos003b", "btfsoloa", "btfsolob", "gun2", "lobcloth", "ptcegg", "dinotrex", "dinoptera", "ptcbodya",
    "ptcwinga", "body2", "tentaclea", "linez", "b6", "c6", "cos004d", "uchiwa", "lollipop", "roripoppu", "uchiwa01", "lollipop01", "roripoppu01", "body01",
    "body02", "body03", "bunny", "playboy", "bunny01", "playboy01", "costume", "costume01", "ear", "candy", "snkcandy", "sensu", "maisensu", "maisens", "maisensu_r",
    "maisensu_l", "maisensu_m", "snkcandy_1", "candy01", "candy02", "bikini", "swim", "swimsuit", "bikini1", "bikini01", "swimsuit01", "swimsuit1", "butterfly",
    "butterfly1", "butterfly01","sensu01", "sensu1", "skirt", "skirt01", "skirt1", "sarong", "sarong01", "sarong1", "pareo", "pareo01", "pareo1", "wrap", "wrap01",
    "wrap1", "bodywrap", "bodywrap01", "bodywrap1", "weight", "dumbbell", "danberu", "syusyu", "lock", "lock1", "lock01", "fusa", "fusa1", "fusa01", "hair1", "hair2",
    "hair3", "extension", "accessory", "fringe", "fringe1", "fringe01", "bangs", "bangs01", "bangs1", "atama", "atama1", "atama01", "maegami", "maegami1", "maegami01",
    "dragon", "dragon01", "dragonbeard01", "dragonbeard1", "dragon1", "hair_a", "christmas", "christmas01", "christmas1", "ornament", "ornament01",
    "k01", "k02", "k03", "k04", "k01_01", "k01_02", "k01_03", "k01_04", "k02_01", "k02_02", "k02_03", "k02_04", "k03_01", "k03_02", "k03_03", "k03_04", "k04_01",
    "k04_02", "k04_03", "k04_04", "bell", "bells", "kane", "bell1", "bell01", "kane1", "kane01", "l01", "l02", "l03", "l04", "l01_01", "l01_02", "l01_03", "l01_04",
    "l02_01", "l02_02", "l02_03", "l02_04", "l03_01", "l03_02", "l03_03", "l03_04", "l04_01", "l04_02", "l04_03", "l04_04", "glove", "lens", "lens1", "lens01",
    "frame", "frame1", "frame01", "i01", "i02", "i03", "i04", "i01_01", "i01_02", "i01_03", "i01_04", "i02_01", "i02_02", "i02_03", "i02_04", "i03_01", "i03_02",
    "i03_03", "i03_04", "i04_01", "i04_02", "i04_03", "i04_04", "j01", "j02", "j03", "j04", "j01_01", "j01_02", "j01_03", "j01_04", "j02_01", "j02_02", "j02_03",
    "j02_04", "j03_01", "j03_02", "j03_03", "j03_04", "j04_01", "j04_02", "j04_03", "j04_04", "a05", "b05", "c05", "g05", "d05", "e05", "f05", "h05", "i05", "j05",
    "k05", "l05", "katana1", "katana01", "weapon1", "weapon01", "katana2", "katana02", "weapon2", "weapon02", "pants1", "f01_gem", "gold", "gold01", "gold1",
    "transformation", "transformation1", "transformation01", "particle", "particle1", "particle01", "effect", "effect1", "effect01", "golden", "golden1", "golden01",
    "glow", "glow1", "glow01", "eye01", "eye02", "eye_01", "eye_02", "eye1", "eye2", "face2", "face02", "face_2", "face_02", "helmet2", "helmet02", "helmet_2", "helmet_02",
    "line02", "line2", "line_2", "line_02", "body2", "body02", "body_2", "body_02", "ribbon2", "ribbon02", "ribbon_2", "ribbon_02", "body_x", "balade", "h01_gem",
    "cloth", "clothn1", "clothn2", "clothn3", "clothn4", "cloth1", "cloth2", "cloth3", "cloth4", "cloth01", "cloth02", "cloth03", "cloth04", "bodyz",
    "grove", "grove01", "grove02", "grove03", "mole",
};

static std::vector<std::string> anim_strings = { "CHRSEL", "ENT", "WIN", "LOSE", "FATAL_FINISH", "DEFAULT", "DAMAGE", "ATTACK_GUARD", "ATTACK_A",
                                               "ATTACK_O", "LIP_BASE", "EYE_UP", "EYE_DOWN", "EYE_LEFT", "EYE_RIGHT", "FATAL",
                                               "DAMAGE_A", "DAMAGE_O", "DAMAGE_U", "EYE_WINK", "CMN", "MENU_STAND", "GREET", "TALK", "SAD",
                                               "BORE", "SURPRISE", "GLAD", "CLIF", "LIP_OPEN", "LIP_LAUGH", "LIP_SAD", "RELAX", "LIP_BORING",
                                               "LIP_ANGRY", "LIP_SURPRISE", "LIP_DISAPPOINTING", "LIP_LAUGH_a", "LIP_SAD_a",
                                                "APPEAL", "APPEAL_0", "APPEAL_1", "APPEAL_2", "APPEAL_3", "APPEAL_4", "APPEAL_5",
                                                "APPEAL_6", "APPEAL_7", "APPEAL_8", "APPEAL_9", "APPEAL_10", "DEFAULT_MOUTH", "ATTACK_MOUTH",
                                                "BB", "BB_0", "BB_1", "BB_2", "BB_3", "BB_4", "BB_5", "BB_6", "BB_7",  "BB_8", "BB_9", "BB_10"
                                               };

static const std::vector<std::string> small_dictionary_stages =
{
    "metal", "steel", "building", "building_stone", "floor", "lantern", "sky", "rock", "roof", "stone",
    "wall", "wood", "bike", "box_w", "brick", "grid", "ground", "litter", "tank", "iron", "light", "candle", "far", "door",
    "glass", "ivy", "speaker", "alpha", "booth", "cloth", "column", "decor", "food", "dolphin", "road", "room", "scaffold", "scrap", "tail",
    "tree",  "palm", "palmtree", "palm_tree", "window", "fence", "other", "orther", "machine", "body", "ball", "part", "parts", "chair",
    "lens", "flower", "gate", "gates", "grass", "sand", "water", "board", "dish", "label", "menu", "photo", "pillar", "shelf", "shelves",
    "speakers", "table", "wire", "fabric", "house", "obj", "rpg", "rpg_all", "display","frame", "hall", "grs", "a", "b", "c", "cover", "lcd",
    "rtm", "sign", "mountain", "sea", "seaside", "high", "high00",
    "a01", "a02", "a03", "a04", "b01", "b02", "b03", "b04", "c01", "c02", "c03", "c04", "d01", "d02", "d03", "d04", "e01", "e02", "e03", "e04",
    "f01", "f02", "f03", "f04", "g01", "g02", "g03", "g04", "woodbox", "bird", "dolphin", "seagull", "rte","wood_box", "wood_board", "woodboard",
    "coconut", "cocnut", "flatcar", "melon", "melon_break", "box", "fruit", "wmelon", "palm", "volley", "net", "volleynet", "volley_net", "surf", "surfboard",
    "shop", "ocean", "barrel", "rubber", "watermelon", "trade", "trades", "boat", "mob",
};

static const std::vector<std::string> texture_types =
{
    "kidsair",
    "kidsalb",
    "kidsnmh",
    "kidsocc",
    "kidsrfr",
    "kidswtm",
    "kidsmm1",
    "kidsmm2",
    "kidsshl",
    "kidss4m",
    "kidsalem",
    "kidsemi",
    "kidsthk",
    "kidsnmb",
    "kidsfur",
    "kidsofs",
};


static const std::vector<std::string> char_codes =
{
    "KAS",
    "BAS",
    "BAY",
    "JAN",
    "LEI",
    "HTM",
    "RYU",
    "HYT",
    "MAR",
    "NIC",
    "KOK",
    "NYO",
    "RID",
    "MIL",
    "ZAC",
    "TIN",
    "HEL",
    "AYA",
    "ELI",
    "LIS",
    "BRA",
    "CRI",
    "RIG",
    "HON",
    "DGO",
    "PHF",
    "MAI",
    "SNK",
    "MOM",
    "RAC",
    "SKD",

    // Put non playable at end
    "CMN",
    "ARD",
    "MPP",
    "SRD",
};


#endif // SMALL_DICTIONARY_H
