"Resource/HudLayout.res"
{
	HudHealth
	{
		"fieldName"		"HudHealth"
		"xpos"	"16"
		"ypos"	"424"
		"wide"	"50"
		"tall"  "44"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"
		
		"text_xpos" "8"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}

	TargetID
	{
		"fieldName" "TargetID"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	
	HudVoiceSelfStatus
    {
		"fieldName" "HudVoiceSelfStatus"
		"visible" "1"
		"enabled" "1"
		"xpos" "r42"    [$WIN32]
		"ypos" "355"    [$WIN32]
		"wide" "32"
		"tall" "32"
    }
 
    HudVoiceStatus
    {
		"fieldName" "HudVoiceStatus"
		"visible" "1"
		"enabled" "1"
		"xpos"	"r177"
		"ypos"	"-45"
		"zpos"	"100"
		"wide"	"175"
		"tall"	"400"
               
		"item_wide"	"170"
		"item_tall"	"16"
               
		"fade_in_time"	"0.03"
		"fade_out_time"	"0.2"      
               
		"show_avatar"	"1"
		"avatar_xpos"	"10"
		"avatar_ypos"	"2"
		"avatar_wide"	"12"
		"avatar_tall"	"12"
               
		"show_dead_icon"	"1"
		"dead_xpos"	"1"
		"dead_ypos"	"0"
		"dead_wide"	"16"
		"dead_tall"	"16"
               
		"show_voice_icon"	"0"
               
		"text_xpos"	"34"
    }
	
	HudSuit
	{
		"fieldName"		"HudSuit"
		"xpos"	"68"
		"ypos"	"424"
		"wide"	"50"
		"tall"  "44"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"

		"text_xpos" "8"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}

	HudAmmo
	{
		"fieldName" "HudAmmo"
		"xpos"	"r90"
		"ypos"	"424"
		"wide"	"75"
		"tall"  "44"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"

		"text_xpos" "6"
		"text_ypos" "5"
		"digit_xpos" "6"
		"digit_ypos" "10"
		"digit2_xpos" "40"
		"digit2_ypos" "16"
	}

	HudAmmoSecondary
	{
		"fieldName" "HudAmmoSecondary"
		"xpos"	"r78"
		"ypos"	"424"
		"wide"	"55"
		"tall"  "44"
		"visible" "1"
		"enabled" "1"

		"PaintBackgroundType"	"2"
		
		"text_xpos" "6"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}
	
	HudSuitPower
	{
		"fieldName" "HudSuitPower"
		"visible" "1"
		"enabled" "1"
		"xpos"	"16"
		"ypos"	"388"
		"wide"	"102"
		"tall"	"26"
		
		"AuxPowerLowColor" "255 0 0 220"
		"AuxPowerHighColor" "255 220 0 220"
		"AuxPowerDisabledAlpha" "70"

		"BarInsetX" "8"
		"BarInsetY" "15"
		"BarWidth" "92"
		"BarHeight" "4"
		"BarChunkWidth" "6"
		"BarChunkGap" "1"

		"text_xpos" "8"
		"text_ypos" "4"
		"text2_xpos" "8"
		"text2_ypos" "22"
		"text2_gap" "10"

		"PaintBackgroundType"	"2"
	}
	
	HudFlashlight
	{
		"fieldName" "HudFlashlight"
		"visible" "0"
		"enabled" "1"
		"xpos"	"16"
		"ypos"	"370"
		"wide"	"102"
		"tall"	"20"
		
		"text_xpos" "8"
		"text_ypos" "6"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}
	
	HudDamageIndicator
	{
		"fieldName" "HudDamageIndicator"
		"visible" "1"
		"enabled" "1"
		"DmgColorLeft" "255 0 0 0"
		"DmgColorRight" "255 0 0 0"
		
		"dmg_xpos" "30"
		"dmg_ypos" "100"
		"dmg_wide" "36"
		"dmg_tall1" "240"
		"dmg_tall2" "200"
	}

	HudZoom
	{
		"fieldName" "HudZoom"
		"visible" "1"
		"enabled" "1"
		"Circle1Radius" "66"
		"Circle2Radius"	"74"
		"DashGap"	"16"
		"DashHeight" "4"
		"BorderThickness" "88"
	}

	HudWeaponSelection
	{
		"fieldName" "HudWeaponSelection"
		"ypos" 	"16"
		"visible" "1"
		"enabled" "1"
		"SmallBoxSize" "32"
		"LargeBoxWide" "162"
		"LargeBoxTall" "72"
		"BoxGap" "4"
		"SelectionNumberXPos" "4"
		"SelectionNumberYPos" "4"
		"SelectionGrowTime"	"0.4"
		"TextYPos" "54"
	}

	HudCrosshair
	{
		"fieldName" "HudCrosshair"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudDeathNotice
	{
		"fieldName" "HudDeathNotice"
		"visible" "1"
		"enabled" "1"
		"xpos"	 "r640"
		"ypos"	 "20"
		"wide"	 "628"
		"tall"	 "468"

		"MaxDeathNotices" "3"
		"LineHeight"	  "36"
		"RightJustify"	  "1"	// If 1, draw notices from the right
		"SpectatorYPosAdd"	"40" //This moves the panel a specified amount of times on the spectator menu. That way, everything looks pretty.

		"TextFont"				"DefaultDeathNotice"
	}

	HudVehicle
	{
		"fieldName" "HudVehicle"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	ScorePanel
	{
		"fieldName" "ScorePanel"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudTrain
	{
		"fieldName" "HudTrain"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMOTD
	{
		"fieldName" "HudMOTD"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudMessage
	{
		"fieldName" "HudMessage"
		"visible" "1"
		"enabled" "1"
		"wide"	 "f0"
		"tall"	 "480"
	}

	HudMenu
	{
		"fieldName" "HudMenu"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudCloseCaption
	{
		"fieldName" "HudCloseCaption"
		"visible"	"1"
		"enabled"	"1"
		"xpos"		"c-250"
		"ypos"		"276"
		"wide"		"500"
		"tall"		"136"

		"BgAlpha"	"128"

		"GrowTime"		"0.25"
		"ItemHiddenTime"	"0.2"  // Nearly same as grow time so that the item doesn't start to show until growth is finished
		"ItemFadeInTime"	"0.15"	// Once ItemHiddenTime is finished, takes this much longer to fade in
		"ItemFadeOutTime"	"0.3"

	}

	HudHistoryResource
	{
		"fieldName" "HudHistoryResource"
		"visible" "0"
		"enabled" "1"
		"xpos"	"r252"
		"ypos"	"50"
		"wide"	 "248"
		"tall"	 "360"

		"history_gap"	"35"
		"icon_inset"	"28"
		"text_inset"	"26"
		"NumberFont"	"HudNumbersSmall"
	}

	HudGeiger
	{
		"fieldName" "HudGeiger"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HUDQuickInfo
	{
		"fieldName" "HUDQuickInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudWeapon
	{
		"fieldName" "HudWeapon"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}
	HudAnimationInfo
	{
		"fieldName" "HudAnimationInfo"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudPredictionDump
	{
		"fieldName" "HudPredictionDump"
		"visible" "1"
		"enabled" "1"
		"wide"	 "640"
		"tall"	 "480"
	}

	HudHintDisplay
	{
		"fieldName"	"HudHintDisplay"
		"visible"	"0"
		"enabled" "1"
		"xpos"	"r120"
		"ypos"	"r340"
		"wide"	"100"
		"tall"	"200"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_xgap"	"8"
		"text_ygap"	"8"
		"TextColor"	"255 170 0 220"

		"PaintBackgroundType"	"2"
	}

	HudSquadStatus
	{
		"fieldName"	"HudSquadStatus"
		"visible"	"1"
		"enabled" "1"
		"xpos"	"r120"
		"ypos"	"380"
		"wide"	"104"
		"tall"	"46"
		"text_xpos"	"8"
		"text_ypos"	"34"
		"SquadIconColor"	"255 220 0 160"
		"IconInsetX"	"8"
		"IconInsetY"	"0"
		"IconGap"		"24"

		"PaintBackgroundType"	"2"
	}

	HudPoisonDamageIndicator
	{
		"fieldName"	"HudPoisonDamageIndicator"
		"visible"	"0"
		"enabled" "1"
		"xpos"	"16"
		"ypos"	"346"
		"wide"	"136"
		"tall"	"38"
		"text_xpos"	"8"
		"text_ypos"	"8"
		"text_ygap" "14"
		"TextColor"	"255 170 0 220"
		"PaintBackgroundType"	"2"
	}
	HudCredits
	{
		"fieldName"	"HudCredits"
		"TextFont"	"Default"
		"visible"	"1"
		"xpos"	"0"
		"ypos"	"0"
		"wide"	"640"
		"tall"	"480"
		"TextColor"	"255 255 255 192"

	}
	"HudChat"
	{
		"ControlName"		"EditablePanel"
		"fieldName" 		"HudChat"
		"visible" 		"1"
		"enabled" 		"1"
		"xpos"			"10"
		"ypos"			"275"
		"wide"	 		"320"
		"tall"	 		"120"
		"PaintBackgroundType"	"2"
	}
	
	AchievementNotificationPanel	
	{
		"fieldName"	"AchievementNotificationPanel"
		"visible"	"0"
		"enabled"	"0"
	}
	
	HudHintKeyDisplay
	{
		"fieldName"	"HudHintKeyDisplay"
		"visible"	"0"
		"enabled" 	"0"
	}
	
	HUDAutoAim
	{
		"fieldName"	"HUDAutoAim"
		"visible"	"0"
		"enabled"	"0"
	}	
	
	HudHDRDemo
	{
		"fieldName"	"HudHDRDemo"
		"visible"	"0"
		"enabled"	"0"
	}
	
	HudCommentary
	{
		"fieldName"	"HudCommentary"
		"visible"	"0"
		"enabled"	"0"
	}
	
	"CHudVote"
	{
		"fieldName"		"CHudVote"
		"visible"		"0"
		"enabled"		"0"
	}
	
	TeamDisplay
	{
		"fieldName" "TeamDisplay"
	    "visible" "0"
	    "enabled" "1"
		"xpos"	"c-40"
		"ypos"	"458"
	    "wide" "200"
	    "tall" "60"
	    "text_xpos" "8"
	    "text_ypos" "2"
	}
	
	HudTimer
	{
		"fieldName"   "HudTimer"
		"xpos"	"c-38"
		"ypos"	"428"
		"wide"  "84"
		"tall"  "27"
		"visible" "0"
		"enabled" "1"
		"PaintBackgroundType" "2"
		"digit_xpos" "2"
		"digit_ypos" "-4"
	}
	
	HudScoreRed
	{
		"fieldName"		"HudScoreRed"
		"xpos"	"c-144"
		"ypos"	"426"
		"wide"  "102"
		"tall"  "44"
		"visible" "0"
		"enabled" "1"

		"PaintBackgroundType"	"2"
		
		"text_xpos" "8"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}
	
	HudScoreBlue
	{
		"fieldName"		"HudScoreBlue"
		"xpos"	"c54"
		"ypos"	"426"
		"wide"  "102"
		"tall"  "44"
		"visible" "0"
		"enabled" "1"

		"PaintBackgroundType"	"2"
		
		"text_xpos" "4"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}
	
	HudScoreFFA
	{
		"fieldName"		"HudScoreFFA"
		"xpos"	"120"
		"ypos"	"424"
		"wide"  "102"
		"tall"  "44"
		"visible" "0"
		"enabled" "1"

		"PaintBackgroundType"	"2"
		
		"text_xpos" "4"
		"text_ypos" "5"
		"digit_xpos" "4"
		"digit_ypos" "10"
	}
}