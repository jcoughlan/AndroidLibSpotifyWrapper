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
static string s_current_uri;
static bool s_is_playing = false;
static sp_playlistcontainer *playlistContainer = NULL;
static bool s_is_waiting_for_metadata = false;
static bool s_play_after_loaded = false;
const char *g_listname;

static void on_pause();
static void on_play();
static void on_starred();
static void on_unstarred();
static void set_star(bool is_starred, sp_session *session, sp_track *track);
static void pl_tracks_added(sp_playlist *pl, sp_track * const * tracks,
		int num_tracks, int position, void *userdata);
static void container_loaded(sp_playlistcontainer *pc, void *userdata);

//playlist track callback
static sp_playlist_callbacks pl_callbacks = { pl_tracks_added, };
static void pl_tracks_added(sp_playlist *pl, sp_track * const * tracks,
		int num_tracks, int position, void *userdata) {
	list<int> int_params;
	int_params.push_back(num_tracks);
	list < string > string_params;
	log("JLOG: pltracks %s", sp_playlist_name(pl));

	/*log("SENDING");

	 string a = "a";
	 string b = "b";
	 JNIEnv *env;
	 jclass classLibSpotify = find_class_from_native_thread(&env);

	 log("MEHTOD Id");
	 jmethodID methodId = env->GetStaticMethodID(classLibSpotify,
	 "onTrackReceived", "Ljava/lang/String;)V");
	 log("after");
	 //	env->CallStaticVoidMethod(classLibSpotify, methodId,L, b);
	 log("deleting");
	 env->DeleteLocalRef(classLibSpotify);
	 log("SENT");*/

	addTask(on_pltracks_added, "on_pltracks", int_params);
}

void on_pltracks_added(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	log("\t JLOG:%d tracks added \n", int_params.front());
}

static void playlist_metadata_updated(sp_playlist *pl, void *userdata) {
	list < string > string_params;

	if (sp_playlist_is_loaded(pl))
		string_params.push_back(sp_playlist_name(pl));
	else
		string_params.push_back("playlist not loaded");
	sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);
	addTask(on_playlist_added, "on_playlist", string_params);

}
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
		int position, void *userdata) {
	//pl_callbacks.tracks_added = NULL;
	pl_callbacks.playlist_metadata_updated = &playlist_metadata_updated;
	sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

}

void on_playlist_added(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {

	string s = string_params.front();

	bool success = true;
	sp_error error = (sp_error) 0;
	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);

	jmethodID methodId = env->GetStaticMethodID(class_libspotify,
			"onPlaylistNameReceived", "(Ljava/lang/String;)V");
	env->CallStaticVoidMethod(class_libspotify, methodId,
			env->NewStringUTF(s.c_str()));
	env->DeleteLocalRef(class_libspotify);

}

static void container_loaded(sp_playlistcontainer *pc, void *userdata) {
	addTask(on_container_loaded, "token_lost:pause");
}
static sp_playlistcontainer_callbacks pc_callbacks = { &playlist_added, NULL,
		NULL, &container_loaded };

void on_container_loaded(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_playlistcontainer_remove_callbacks(playlistContainer, &pc_callbacks,
			NULL);

	for (int i = 0; i < sp_playlistcontainer_num_playlists(playlistContainer);
			++i) {
		sp_playlist *pl = sp_playlistcontainer_playlist(playlistContainer, i);
	}

}

void fetchallplaylistcontainers(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	sp_playlistcontainer *pc = sp_session_playlistcontainer(session);
	;
	playlistContainer = pc;
	sp_playlistcontainer_add_callbacks(pc, &pc_callbacks, NULL);
}

void login(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	if (session == NULL) {
		log("Logged in before session was initialized");

		exitl("Logged in before session was initialized");
	} else
		log("Login task");
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

void on_allplaylistcontainers_fetched(list<int> int_params,
		list<string> string_params, sp_session *session, sp_track *track) {
	call_static_void_method("onAllPlaylistContainersFetched");
}

void on_logged_in(list<int> int_params, list<string> string_params,
		sp_session *session, sp_track *track) {
	sp_error error = (sp_error) int_params.front();
	bool success = (SP_ERROR_OK == error) ? true : false;

	JNIEnv *env;
	jclass class_libspotify = find_class_from_native_thread(&env);

	jmethodID methodId = env->GetStaticMethodID(class_libspotify, "onLogin",
			"(ZLjava/lang/String;)V");

	env->CallStaticVoidMethod(class_libspotify, methodId, success,
			env->NewStringUTF(sp_error_message(error)));
	env->DeleteLocalRef(class_libspotify);
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
