package com.example.classes;

public class AppInstance {
	public static AppInstance currentInstance = new AppInstance();
	
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
