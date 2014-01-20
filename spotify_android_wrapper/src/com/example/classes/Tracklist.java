package com.example.classes;

import java.util.ArrayList;

public class Tracklist {

	private ArrayList<Track> tracklist = new ArrayList<Track>();

	public void AddTrack(String title, String albumName, String artistName,
			String uri) {
		Track track = new Track(title, albumName, artistName, uri);
		tracklist.add(track);
	}

	public ArrayList<Track> GetTrackListArray() {
		return tracklist;
	}

	public Track GetTrack(int index) {
		return tracklist.get(index);
	}

	public int GetNumTracks() {
		return tracklist.size();
	}
}
