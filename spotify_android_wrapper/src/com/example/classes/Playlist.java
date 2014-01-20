package com.example.classes;

import java.util.ArrayList;

public class Playlist {
	private String title = "no title";
	private Tracklist tracklist = new Tracklist();

	public void SetTitle(String text) {
		title = text;
	}

	public String GetTitle() {
		return title;
	}

	public void AddTrack(String title, String albumName, String artistName, String uri) {
		tracklist.AddTrack(title, albumName, artistName, uri);
	}

	public Tracklist GetTrackList() {
		return tracklist;
	}
	
	public int Length()
	{
		return tracklist.GetTrackListArray().size();
	}

}
