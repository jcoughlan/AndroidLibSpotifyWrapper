/*
 Copyright (c) 2012, Spotify AB
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of Spotify AB nor the names of its contributors may
 be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL SPOTIFY AB BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Tasks that can be added to the queue of tasks running on the libspotify thread
 */
#include "tasks.h"
#include "run_loop.h"
#include "jni_glue.h"
#include "logger.h"
#include "sound_driver.h"

static int s_player_position = 0;
static int numPlaylistsLeftToLoad = 0;
static string s_current_uri;
static bool s_is_playing = false;
static sp_playlistcontainer *playlistContainer = NULL;
static bool s_is_waiting_for_metadata = false;
static bool s_play_after_loaded = false;
const char *g_listname;
static sp_session* currentSession = NULL;
static void on_pause();
static void on_play();
static void on_starred();
static void on_unstarred();
static void set_star(bool is_starred, sp_session *session, sp_track *track);
static void pl_tracks_added(sp_playlist *pl, sp_track * const * tracks,
		int num_tracks, int position, void *userdata);
static void container_loaded(sp_playlistcontainer *pc, void *userdata);

//playlist track callback
static sp_playlist_callbacks pl_callbacks = { };

//not currently used
static void pl_tracks_added(sp_playlist *pl, sp_track * const * tracks,
		int num_tracks, int position, void *userdata) {
}

//not currently used
void on_pltracks_added(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
}

void check_connection(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {

	char* currentState;
	if (sp_session_connectionstate(currentSession)
			== sp_connectionstate::SP_CONNECTION_STATE_LOGGED_IN) {
		currentState = (char*) ("STATE_LOGGED_IN");
	} else if (sp_session_connectionstate(currentSession)
			== sp_connectionstate::SP_CONNECTION_STATE_DISCONNECTED) {
		currentState = (char*) ("STATE_DISCONNECTED");
	} else if (sp_session_connectionstate(currentSession)
			== sp_connectionstate::SP_CONNECTION_STATE_LOGGED_OUT) {
		currentState = (char*) ("STATE_LOGGED_OUT");
	} else if (sp_session_connectionstate(currentSession)
			== sp_connectionstate::SP_CONNECTION_STATE_OFFLINE) {
		currentState = (char*) ("STATE_OFFLINE");
	}

	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);
	jstring arg1 = env->NewStringUTF(currentState);

	jmethodID methodId = env->GetStaticMethodID(class_libspotify,
			"onConnectionStatusReceived", "(Ljava/lang/String;)V");

	env->CallStaticVoidMethod(class_libspotify, methodId, arg1);

	env->DeleteLocalRef(class_libspotify);
	env->DeleteLocalRef(arg1);

}

void logout(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_session_logout(session);
}
void on_logged_out(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_error error = (sp_error) int_params.front();
	bool success = (SP_ERROR_OK == error) ? true : false;

	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);
	jstring arg1 = env->NewStringUTF(sp_error_message(error));
	jmethodID methodId = env->GetStaticMethodID(class_libspotify, "onLogout",
			"(ZLjava/lang/String;)V");

	env->CallStaticVoidMethod(class_libspotify, methodId, success, arg1);
	env->DeleteLocalRef(class_libspotify);
	env->DeleteLocalRef(arg1);
}
void on_tracks_added_java_callback(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	sp_playlist* pl = NULL;
	int numPlaylists = sp_playlistcontainer_num_playlists(playlistContainer);
	for (int i = 0; i < numPlaylists; i++) {
		sp_playlist * pl_c = sp_playlistcontainer_playlist(playlistContainer,
				i);
		if (sp_playlist_name(pl_c) == string_params.front()) {
			pl = pl_c;
		}
	}
	if (!pl)
		return;

	int numTracks = sp_playlist_num_tracks(pl);
	for (int i = 0; i < numTracks; i++) {
		sp_track *t = sp_playlist_track(pl, i);

		if (sp_track_is_loaded(t)) {
			JNIEnv *env;
			jclass classLibSpotify = find_class_from_native_thread(&env);
			jmethodID methodId =
					env->GetStaticMethodID(classLibSpotify, "onTrackReceived",
							"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

			sp_link* sl = sp_link_create_from_track(t, 0);
			char buffer[30];
			int x = sp_link_as_string(sl, buffer, 30);
			jstring StringArg1 = env->NewStringUTF(sp_track_name(t));
			jstring StringArg2 = env->NewStringUTF(sp_playlist_name(pl));
			jstring StringArg3 = env->NewStringUTF(
					sp_artist_name(sp_track_artist(t, 0)));
			jstring StringArg4 = env->NewStringUTF(
					sp_album_name(sp_track_album(t)));
			jstring StringArg5 = env->NewStringUTF(buffer);

			env->CallStaticVoidMethod(classLibSpotify, methodId, StringArg1,
					StringArg2, StringArg3, StringArg4, StringArg5);

			env->DeleteLocalRef(StringArg1);
			env->DeleteLocalRef(StringArg2);
			env->DeleteLocalRef(StringArg3);
			env->DeleteLocalRef(StringArg4);
			env->DeleteLocalRef(StringArg5);
			env->DeleteLocalRef(classLibSpotify);
			sp_track_release(t);

		}
	}
	sp_playlist_release(pl);
}
void on_playlist_added_java_callback(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	sp_playlist* pl = NULL;
	int numPlaylists = sp_playlistcontainer_num_playlists(playlistContainer);
	for (int i = 0; i < numPlaylists; i++) {
		sp_playlist * pl_c = sp_playlistcontainer_playlist(playlistContainer,
				i);
		if (sp_playlist_name(pl_c) == string_params.front()) {
			pl = pl_c;
		}
	}
	if (!pl)
		return;

	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);
	jstring arg1 = env->NewStringUTF(string_params.front().c_str());
	int timeout = 20;
	byte image_id[20];
	sp_playlist_get_image(pl, image_id);
	sp_image* image = sp_image_create(currentSession, image_id);

	while (!sp_image_is_loaded(image)) {
		sp_session_process_events(currentSession, &timeout);
	}

	size_t size;
	const void* image_data = sp_image_data(image, &size);
	jbyteArray result;
	result = env->NewByteArray(size);

	env->SetByteArrayRegion(result, 0, size, (const jbyte*) image_data);

	jmethodID methodId = env->GetStaticMethodID(class_libspotify,
			"onPlaylistReceived", "([BLjava/lang/String;)V");

	env->CallStaticVoidMethod(class_libspotify, methodId, result, arg1);

	env->DeleteLocalRef(class_libspotify);
	env->DeleteLocalRef(arg1);
	env->DeleteLocalRef(result);
	sp_image_release(image);
	image_data = NULL;
	numPlaylistsLeftToLoad--;
	if (numPlaylistsLeftToLoad - 1 == 0) {
		//we are finished
		log("PLAYLISTS FINISHED");
		JNIEnv *env;
		jclass classLibSpotify = find_class_from_native_thread(&env);
		jmethodID methodId = env->GetStaticMethodID(classLibSpotify,
				"onAllPlaylistsAndTracksReceived", "()V");
		env->CallStaticVoidMethod(classLibSpotify, methodId);
		env->DeleteLocalRef(classLibSpotify);

	}
	addTask(on_tracks_added_java_callback, "tracks java callback",
			string_params);

}

static void playlist_metadata_updated(sp_playlist *pl, void *userdata) {
	list < string > string_params;
	list<int> int_params;
	if (sp_playlist_is_loaded(pl)) {

		int numTracks = sp_playlist_num_tracks(pl);
		for (int i = 0; i < numTracks; i++) {
			sp_track *t = sp_playlist_track(pl, i);
			if (!sp_track_is_loaded(t)) {
				return;
			}
		}
		sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);

		string pl_name = sp_playlist_name(pl);
		string_params.push_back(pl_name);
		addTask(on_playlist_added_java_callback, "java callback",
				string_params);

		//check if thats the last one

	}
}
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
		int position, void *userdata) {
	pl_callbacks.tracks_added = NULL;
	pl_callbacks.playlist_metadata_updated = &playlist_metadata_updated;
	pl_callbacks.playlist_state_changed = NULL;
	sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
}

static void container_loaded(sp_playlistcontainer *pc, void *userdata) {
	numPlaylistsLeftToLoad = sp_playlistcontainer_num_playlists(pc);
	addTask(on_container_loaded, "on_container_loaded");

}

static sp_playlistcontainer_callbacks pc_callbacks = { &playlist_added, NULL,
		NULL, &container_loaded };

//just to remove all our container callbacks
void on_container_loaded(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_playlistcontainer_remove_callbacks(playlistContainer, &pc_callbacks,
			NULL);
}

//fetches all playlists containers (pointers to the playlists/no real data)
void fetchallplaylistcontainers(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	sp_playlistcontainer *pc = sp_session_playlistcontainer(session);

	playlistContainer = pc;
	sp_playlistcontainer_add_callbacks(pc, &pc_callbacks, NULL);
}

static void search_complete(sp_search *result, void *userdata) {
	sp_search* search = result;
	int numTracks = sp_search_num_tracks(search);
	int numArtists = sp_search_num_artists(search);
	int numPlaylists = sp_search_num_playlists(search);
	int numAlbums = sp_search_num_albums(search);

	for (int i = 0; i < numTracks; i++) {
		sp_track* track = sp_search_track(search, i);
		log("Search Track %s", sp_track_name(track));
	}
	for (int i = 0; i < numArtists; i++) {
		sp_artist* artist = sp_search_artist(search, i);
		log("Search Artist %s", sp_artist_name(artist));
	}
	for (int i = 0; i < numPlaylists; i++) {
		sp_playlist* pl = sp_search_playlist(search, i);
		log("Search Playlist %s", sp_playlist_name(pl));
	}
	for (int i = 0; i < numAlbums; i++) {
		sp_album* album = sp_search_album(search, i);
		log("Search Album %s", sp_album_name(album));
	}

}

void search(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	int params[] = { 0, 0, 0, 0 };

	int index = 0;
	list<int>::iterator it = int_params.begin();

	sp_search * searchObj = sp_search_create(session,
			string_params.front().c_str(), 0, *(it), 0, *(++it), 0, *(++it), 0,
			*(++it), sp_search_type::SP_SEARCH_STANDARD, &search_complete,
			NULL);
	log("Search obj created");

}

//fetch album info (only returns the byte array for images right now)
void fetchalbuminfo(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	/*sp_link* tr_link = sp_link_create_from_string(
	 string_params.front().c_str());
	 sp_track* tr = sp_link_as_track(tr_link);
	 sp_album* album = sp_track_album(tr);
	 if (sp_album_is_loaded(album)) {
	 int timeout = 0;
	 const byte* image_id = sp_album_cover(album, SP_IMAGE_SIZE_NORMAL);

	 sp_image* image = sp_image_create(session, image_id);
	 while (!sp_image_is_loaded(image)) {
	 sp_session_process_events(session, &timeout);
	 }
	 size_t size;
	 JNIEnv *env;
	 jclass classLibSpotify = find_class_from_native_thread(&env);
	 const void* image_data = sp_image_data(image, &size);
	 jbyteArray result = env->NewByteArray(size);
	 env->SetByteArrayRegion(result, 0, size, (const jbyte*) image_data);
	 jmethodID methodId = env->GetStaticMethodID(classLibSpotify,
	 "onAlbumCoverReceived", "([B)V");
	 env->CallStaticVoidMethod(classLibSpotify, methodId, result);
	 env->DeleteLocalRef(classLibSpotify);
	 env->DeleteLocalRef(result);
	 } else {
	 }*/
}

void login(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	if (session == NULL) {
		exitl("Logged in before session was initialized");
	}
	currentSession = session;
	string username = string_params.front();
	string password = string_params.back();
	sp_session_login(session, username.c_str(), password.c_str(), true, NULL);
}

static void play_track(sp_session *session, sp_track *track) {
//unmute(opensl);
	sp_session_player_play(session, true);
	s_is_playing = true;
	on_play();
}

// Loads a track and assumes that the metadata is available
static void load_and_play_track(sp_session *session, sp_track *track) {
	sp_session_player_load(session, track);
	if (s_play_after_loaded)
		play_track(session, track);
	(sp_track_is_starred(session, track)) ? on_starred() : on_unstarred();
}

// Load the track if the metadata update was concerning the track
void load_and_play_track_after_metadata_updated(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	if (s_is_waiting_for_metadata == true && sp_track_is_loaded(track)) {
		s_is_waiting_for_metadata = false;
		load_and_play_track(session, track);
	}
}

// Loads track if metadata exists, otherwise load the metadata
static void load_track_or_metadata(sp_session *session, sp_track *track,
		const char *uri) {
	if (track != NULL) {
		if (s_is_playing)
			sp_session_player_play(session, false);
		sp_session_player_unload(session);
		sp_track_release(track);
	}
	track = sp_link_as_track(sp_link_create_from_string(uri));
	set_track(track);
	sp_track_add_ref(track);
	s_player_position = 0;
	s_current_uri = uri;

// either the track is already cached and can be used or we need to wait for the metadata callback
	if (sp_track_is_loaded(track)) {
		load_and_play_track(session, track);
	} else
		s_is_waiting_for_metadata = true;
}

// Play a new track. It will only play the song if the previous song was playing
void play_next(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	string uri = string_params.front();
	s_play_after_loaded = s_is_playing;
	load_track_or_metadata(session, track, uri.c_str());
}

// Play or resume the song
void toggle_play(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	string uri = string_params.front();

	if (!s_is_playing) {
		// just resume if its the same uri (current_uri is only set if the track has been loaded into the player)
		if (uri == s_current_uri)
			play_track(session, track);
		else {
			load_track_or_metadata(session, track, uri.c_str());
			s_play_after_loaded = true;
		}
	} else
		pause(int_params, string_params, session, track);
}

void pause(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	if (s_is_playing) {
		s_is_playing = false;
		sp_session_player_play(session, false);
		//mute(opensl);
		on_player_pause(int_params, string_params, session, track);
	}
}

void star(list<int> int_params, list<string> string_params, sp_session *session,
		sp_track *track) {
	set_star(true, session, track);
	on_starred();
}

void unstar(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	set_star(false, session, track);
	on_unstarred();
}

void seek(list<int> int_params, list<string> string_params, sp_session *session,
		sp_track *track) {
	float position = (float) int_params.front() / 100.0;
	int pos_ms = (int) ((float) sp_track_duration(track) * position);

	if (s_is_playing)
		sp_session_player_play(session, false);
	sp_session_player_seek(session, pos_ms);
	if (s_is_playing)
		sp_session_player_play(session, true);
	s_player_position = pos_ms / 1000;
}

void on_player_position_changed(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	s_player_position++;

	int total_length = sp_track_duration(track) / 1000;
	float percentage = (float) s_player_position / (float) total_length;

	JNIEnv *env;
	jclass classLibSpotify = find_class_from_native_thread(&env);

	jmethodID methodId = env->GetStaticMethodID(classLibSpotify,
			"onPlayerPositionChanged", "(F)V");
	env->CallStaticVoidMethod(classLibSpotify, methodId, percentage);
	env->DeleteLocalRef(classLibSpotify);
}

void on_end_of_track(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	call_static_void_method("onEndOfTrack");
}

void on_logged_in(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_error error = (sp_error) int_params.front();
	bool success = (SP_ERROR_OK == error) ? true : false;

	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);
	jstring arg1 = env->NewStringUTF(sp_error_message(error));
	jmethodID methodId = env->GetStaticMethodID(class_libspotify, "onLogin",
			"(ZLjava/lang/String;)V");

	env->CallStaticVoidMethod(class_libspotify, methodId, success, arg1);
	env->DeleteLocalRef(class_libspotify);
	env->DeleteLocalRef(arg1);
}

void on_player_pause(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	on_pause();
}

void on_player_end_of_track(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	call_static_void_method("onPlayerEndOfTrack");
}

void destroy(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_session_release(session);
	destroy_audio_player();
}

static void on_pause() {
	call_static_void_method("onPlayerPause");
}
static void on_play() {
	call_static_void_method("onPlayerPlay");
}
static void on_starred() {
	call_static_void_method("onTrackStarred");
}
static void on_unstarred() {
	log("Unstarred now");
	call_static_void_method("onTrackUnStarred");
}

static void set_star(bool is_starred, sp_session *session, sp_track *track) {
	if (sp_track_set_starred(session, &track, 1, is_starred) != SP_ERROR_OK)
		exitl("Could not star/unstar the track");
}
