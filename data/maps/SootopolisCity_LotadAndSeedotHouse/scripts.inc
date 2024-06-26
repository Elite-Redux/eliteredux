SootopolisCity_LotadAndSeedotHouse_MapScripts:: @ 822722A
	.byte 0

SootopolisCity_LotadAndSeedotHouse_EventScript_SeedotBrother:: @ 822722B
	special GetSeedotSizeRecordInfo
	lock
	faceplayer
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_PleaseShowMeBigSeedot, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	compare VAR_RESULT, 255
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_CancelShowSeedot
	special CompareSeedotSize
	compare VAR_RESULT, 1
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_NotSeedot
	compare VAR_RESULT, 2
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_SmallSeedot
	compare VAR_RESULT, 3
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_BigSeedot
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_CancelShowSeedot:: @ 8227272
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_DontHaveBigSeedot, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_NotSeedot:: @ 822727C
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_ThatsNotSeedot, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_SmallSeedot:: @ 8227286
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_SeenBiggerSeedot, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_BigSeedot:: @ 8227290
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_GoshMightBeBiggerThanLotad, MSGBOX_DEFAULT
	giveitem ITEM_ELIXIR
	compare VAR_RESULT, FALSE
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_NoRoomForElixir1
	closemessage
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_NoRoomForElixir1:: @ 82272B2
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_BagCrammedFull1, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_LotadBrother:: @ 82272BC
	special GetLotadSizeRecordInfo
	lock
	faceplayer
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_PleaseShowMeBigLotad, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	compare VAR_RESULT, 255
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_CancelShowLotad
	special CompareLotadSize
	compare VAR_RESULT, 1
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_NotLotad
	compare VAR_RESULT, 2
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_SmallLotad
	compare VAR_RESULT, 3
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_BigLotad
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_CancelShowLotad:: @ 8227303
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_DontHaveBigLotad, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_NotLotad:: @ 822730D
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_ThatsNotLotad, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_SmallLotad:: @ 8227317
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_SeenBiggerLotad, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_BigLotad:: @ 8227321
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_WowMightBeBiggerThanSeedot, MSGBOX_DEFAULT
	giveitem ITEM_ELIXIR
	compare VAR_RESULT, FALSE
	goto_if_eq SootopolisCity_LotadAndSeedotHouse_EventScript_NoRoomForElixir2
	closemessage
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_NoRoomForElixir2:: @ 8227343
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_BagCrammedFull2, MSGBOX_DEFAULT
	release
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_SeedotSizeRecord:: @ 822734D
	special GetSeedotSizeRecordInfo
	lockall
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_BiggestSeedotInHistory, MSGBOX_DEFAULT
	releaseall
	end

SootopolisCity_LotadAndSeedotHouse_EventScript_LotadSizeRecord:: @ 822735B
	special GetLotadSizeRecordInfo
	lockall
	msgbox SootopolisCity_LotadAndSeedotHouse_Text_BiggestLotadInHistory, MSGBOX_DEFAULT
	releaseall
	end

SootopolisCity_LotadAndSeedotHouse_Text_PleaseShowMeBigSeedot: @ 8227369
	.string "Do you know the Pokémon Seedot?\n"
	.string "It's hardly ever seen in Sootopolis.\p"
	.string "Anyway, I love big Seedot.\n"
	.string "The bigger the better.\p"
	.string "But my younger brother, he says that\n"
	.string "LOTAD is bigger.\p"
	.string "That's silly.\n"
	.string "SEEDOT has to be bigger than that!\p"
	.string "Huh? Do you have a Seedot with you?\n"
	.string "P-p-please, show me!$"

SootopolisCity_LotadAndSeedotHouse_Text_GoshMightBeBiggerThanLotad: @ 8227480
	.string "{STR_VAR_2} inches!\n"
	.string "Oh, my gosh, this is a big one!\p"
	.string "It might even beat the big Lotad\n"
	.string "my younger brother saw!\p"
	.string "Thanks for showing me.\n"
	.string "This is my thanks!$"

@ Unused
SootopolisCity_LotadAndSeedotHouse_Text_ReceivedPotion1: @ 822750E
	.string "{PLAYER} received a Potion.$"

SootopolisCity_LotadAndSeedotHouse_Text_BagCrammedFull1: @ 8227524
	.string "Hunh?\n"
	.string "Your Bag is crammed full.$"

SootopolisCity_LotadAndSeedotHouse_Text_SeenBiggerSeedot: @ 8227544
	.string "{STR_VAR_2} inches, is it?\p"
	.string "Hmm… I've seen a bigger Seedot\n"
	.string "than this one.$"

SootopolisCity_LotadAndSeedotHouse_Text_ThatsNotSeedot: @ 8227584
	.string "Oh, now this is quite something…\n"
	.string "But it's not a Seedot!$"

SootopolisCity_LotadAndSeedotHouse_Text_DontHaveBigSeedot: @ 82275BC
	.string "You don't have a big Seedot?\n"
	.string "That's too bad…\p"
	.string "If you get a big Seedot, please\n"
	.string "come show me.$"

SootopolisCity_LotadAndSeedotHouse_Text_BiggestSeedotInHistory: @ 8227617
	.string "The biggest Seedot in history!\n"
	.string "{STR_VAR_2}'s {STR_VAR_3}-inch giant!\p"
	.string "A Seedot bigger than a Lotad\n"
	.string "always wanted!$"

SootopolisCity_LotadAndSeedotHouse_Text_PleaseShowMeBigLotad: @ 8227676
	.string "Do you know the Pokémon Lotad?\n"
	.string "It's rarely seen in Sootopolis.\p"
	.string "I love, I mean love, big Lotad!\p"
	.string "My big brother says that Seedot is\n"
	.string "bigger.\p"
	.string "But that's wrong! Everyone knows that\n"
	.string "Lotad is a lot bigger.\p"
	.string "Hunh? Do you have a Lotad?\n"
	.string "P-p-please show me!$"

SootopolisCity_LotadAndSeedotHouse_Text_WowMightBeBiggerThanSeedot: @ 822776C
	.string "{STR_VAR_2} inches!\n"
	.string "Wow, that is big!\p"
	.string "It might be even bigger than the huge\n"
	.string "Seedot my big brother saw.\p"
	.string "Thanks for showing me!\n"
	.string "This is my thanks!$"

@ Unused
SootopolisCity_LotadAndSeedotHouse_Text_ReceivedPotion2: @ 82277F4
	.string "{PLAYER} received a Potion.$"

SootopolisCity_LotadAndSeedotHouse_Text_BagCrammedFull2: @ 822780A
	.string "Hunh?\n"
	.string "Your Bag is crammed full.$"

SootopolisCity_LotadAndSeedotHouse_Text_SeenBiggerLotad: @ 822782A
	.string "{STR_VAR_2} inches?\p"
	.string "Hmm… I've seen a bigger Lotad\n"
	.string "than this one here.$"

SootopolisCity_LotadAndSeedotHouse_Text_ThatsNotLotad: @ 8227867
	.string "Well, isn't this something!\n"
	.string "But it's no Lotad!$"

SootopolisCity_LotadAndSeedotHouse_Text_DontHaveBigLotad: @ 8227896
	.string "Don't you have a big Lotad?\n"
	.string "How disappointing…\p"
	.string "If you get a big Lotad, please\n"
	.string "come show me!$"

SootopolisCity_LotadAndSeedotHouse_Text_BiggestLotadInHistory: @ 82278F2
	.string "The biggest Lotad in history!\n"
	.string "{STR_VAR_2}'s {STR_VAR_3}-inch colossus!\p"
	.string "A Lotad bigger than a Seedot\n"
	.string "always wanted!$"
