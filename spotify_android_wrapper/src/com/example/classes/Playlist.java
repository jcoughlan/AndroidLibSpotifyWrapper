package com.example.classes;

import java.util.ArrayList;

public class Playlist {
	private String title = null;
	private Tracklist tracklist = new Tracklist();

	public void SetTitle(String text) {
		title = text;
	}

	public String GetTitle() {
		return title;
	}

	public void AddTrack(String title) {
		tracklist.AddTrack(title);
	}

	public ArrayList<Track> GetTrackList() {
		return tracklist.GetTrackList();
	}

}
