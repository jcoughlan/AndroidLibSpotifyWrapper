package com.example.classes;

public class Instance {
	public static Instance currentInstance = new Instance();
	
	private Playlist currentPlaylist = null;
	private Track currentTrack = null;
	
	public void SetCurrentPlaylist(Playlist curPlaylist)
	{
		currentPlaylist = curPlaylist;
	}
	
	public Playlist GetCurrentPlaylist()
	{
		return currentPlaylist;
	}
	
	public void SetCurrentTrack(Track tr)
	{
		currentTrack = tr;
	}
	
	public Track GetCurrentTrack()
	{		
		return currentTrack;
	}
}
