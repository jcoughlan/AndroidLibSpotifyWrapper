package com.example.classes;

import java.util.ArrayList;

public class Tracklist {
	
	private ArrayList<Track> tracklist = new ArrayList<Track>();
	
	public void AddTrack(String title)
	{
		Track track = new Track(title,"no album", "no artist", "no uri");
		tracklist.add(track);		
	}
	
	public ArrayList<Track> GetTrackList()
	{
		return tracklist;
	}
	
	public int GetNumTracks()
	{
		return tracklist.size();
	}
}
