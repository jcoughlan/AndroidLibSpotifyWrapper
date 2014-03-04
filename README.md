AndroidLibSpotifyWrapper
========================


******************************************************************************************
		******NOTE******
		
I havent updated this project in a while as i've been working on a seperate private project utilising this library.
In this project I have implemented a huge amount of new features so it's nearly complete. I will be updating this repo sometime near the end of March 2014 with the updated code. In the meantime if you need any help with any calls to the library email me at the address below.


*******************************************************************************************

A libSpotify wrapper for Android

Feel free to contribute to the project!

Based on https://github.com/spotify/psyonspotify libSpotify wrapper. The aim of this project is to implement as many calls to the libspotify library as possible.

FLOW

*Login->ShowPlaylists->SelectPlaylist->ViewSongDetails->PlaySong

The app isnt aimed at providing a particularly good user experience but rather a type of Spotify HelloWorld to showcase the integration with libSpotify, and make it easy for others to copy,  as I don't believe there is a fully fledged libSpotify wrapper available for Android yet.

KNOWN ISSUES:

*Regular Spotify Library crashes-> Not sure why this is happening but its not a huge concern as of yet.
*'Service leaked' Android Error-> Looking into this .
														
DONE SO FAR:

*Track Playback (psyonspotify)
*Track Starring (psyonspotify)
*Login (psyonspotify)
*Basic playlist population
*Tracklist Population
*Play from playlist/tracklist
*Album cover download (using the spotify track link)
*Playlist cover download (if there is one, otherwise use cover for first track on playlist)
*re-initialisation of player

DOING NOW:

memory optimisation, memory release

TO BE DONE:

* Layout (I havent put any effort into this as of yet)
* Everything else! searching, album browsing, all other calls to library etc.

SETUP

Please refer to https://github.com/spotify/psyonspotify for details on how to set up the project

CONTACT

Please email me at coughlan.james@outlook.com if you have any questions.

Readme last updated: 04/03/2014
