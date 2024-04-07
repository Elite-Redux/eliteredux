FortreeCity_House2_MapScripts:: @ 82177CA
	.byte 0

FortreeCity_House2_EventScript_HiddenPowerGiver:: @ 82177CB
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_TM10, FortreeCity_House2_EventScript_ExplainHiddenPower
	call_if_unset FLAG_MET_HIDDEN_POWER_GIVER, FortreeCity_House2_EventScript_Greeting
	msgbox FortreeCity_House2_Text_CoinInWhichHand, MSGBOX_DEFAULT
	multichoice 21, 8, MULTI_RIGHTLEFT, TRUE
	switch VAR_RESULT
	case 1, FortreeCity_House2_EventScript_WrongGuess
	msgbox FortreeCity_House2_Text_CorrectTryAgainWhichHand, MSGBOX_DEFAULT
	multichoice 21, 8, MULTI_RIGHTLEFT, TRUE
	switch VAR_RESULT
	case 1, FortreeCity_House2_EventScript_WrongGuess
	msgbox FortreeCity_House2_Text_CorrectTryAgainWhichHand2, MSGBOX_DEFAULT
	multichoice 21, 8, MULTI_RIGHTLEFT, TRUE
	switch VAR_RESULT
	case 0, FortreeCity_House2_EventScript_WrongGuess
	msgbox FortreeCity_House2_Text_YourHiddenPowerHasAwoken, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM10
	msgbox FortreeCity_House2_Text_ExplainHiddenPower, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_Greeting:: @ 8217862
	msgbox FortreeCity_House2_Text_HiddenPowersArousedByNature, MSGBOX_DEFAULT
	setflag FLAG_MET_HIDDEN_POWER_GIVER
	return

FortreeCity_House2_EventScript_ExplainHiddenPower:: @ 821786E
	msgbox FortreeCity_House2_Text_ExplainHiddenPower, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_WrongGuess:: @ 8217878
	msgbox FortreeCity_House2_Text_YouGuessedWrong, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_SleepTalkTM::
	lock
	faceplayer
	checkitem ITEM_TM49_SLEEP_TALK, 1
	compare VAR_RESULT, 1
	goto_if_eq FortreeCity_House2_EventScript_ReceivedTM49
	msgbox FortreeCity_House2_Text_WantSleepTalk, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq FortreeCity_House2_EventScript_SleepTalkDeclined
	msgbox FortreeCity_Text_GiveTM49, MSGBOX_DEFAULT
	giveitem ITEM_TM49_SLEEP_TALK
	msgbox FortreeCity_Text_HaveTM49, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_ReceivedTM49::
	msgbox FortreeCity_Text_HaveTM49, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_SleepTalkDeclined::
	msgbox FortreeCity_House2_Text_SleepTalkDeclined, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_SleepTalkTaught::
	msgbox FortreeCity_Text_HaveTM49, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_Text_WantSleepTalk:
	.string "Humph! My wife relies on Hidden\n"
	.string "Power to stay awake.\p"
	.string "She should just take a nap like I do,\n"
	.string "and Sleep Talk.\p"
	.string "I can teach your Pokémon how to\n"
	.string "Sleep Talk instead. Interested?$"

FortreeCity_House2_Text_SleepTalkDeclined:
	.string "Oh, fine, fine. You want to stay awake\n"
	.string "with Hidden Power, too…$"

FortreeCity_Text_GiveTM49:
	.string "Ah, an appreciative child!\n"
	.string "Please take this TM.$"

FortreeCity_Text_HaveTM49:
	.string "I've never once gotten my wife's\n"
	.string "coin trick right.\p"
	.string "I would be happy if I got it right\n"
	.string "even as I Sleep Talk…$"

FortreeCity_House2_Text_HiddenPowersArousedByNature: @ 8217882
	.string "People… Pokémon…\p"
	.string "Their hidden powers are aroused by\n"
	.string "living in natural environments…$"

FortreeCity_House2_Text_CoinInWhichHand: @ 82178D6
	.string "Let this old woman see if your hidden\n"
	.string "power has awoken…\p"
	.string "I hold a coin in my hand.\p"
	.string "Now, tell me, have I palmed it in\n"
	.string "the right hand? Or in the left?$"

FortreeCity_House2_Text_CorrectTryAgainWhichHand: @ 821796A
	.string "Oh! Yes, correct!\p"
	.string "We shall try again.\p"
	.string "In which hand have I palmed the coin?\n"
	.string "The right or left?$"

FortreeCity_House2_Text_CorrectTryAgainWhichHand2: @ 82179C9
	.string "Oh! Yes, correct!\p"
	.string "We shall try again.\p"
	.string "In which hand have I palmed the coin?\n"
	.string "The right or left?$"

FortreeCity_House2_Text_YourHiddenPowerHasAwoken: @ 8217A28
	.string "Oh! Splendid!\p"
	.string "I would give you Hidden Power now,\n"
	.string "but you have all TMs already...$"

FortreeCity_House2_Text_ExplainHiddenPower: @ 8217A91
	.string "Hidden Power is a move that changes\n"
	.string "with the Pokémon's IVs.$"

FortreeCity_House2_Text_YouGuessedWrong: @ 8217AC7
	.string "No, too bad.\n"
	.string "You guessed wrong.$"