package com.example.classes;

import java.util.ArrayList;

import android.graphics.Bitmap;

public class Playlist {
	private String title = "no title";
	private Tracklist tracklist = new Tracklist();
	private Bitmap coverBitmap = null;

	public void SetTitle(String text) {
		title = text;
	}

	public String GetTitle() {
		return title;
	}

	public void AddTrack(String title, String albumName, String artistName,
			String uri) {
		tracklist.AddTrack(title, albumName, artistName, uri);
	}

	public Tracklist GetTrackList() {
		return tracklist;
	}

	public int Length() {
		return tracklist.GetTrackListArray().size();
	}

	public void SetCover(Bitmap cover) {
		coverBitmap = cover;
	}

	public Bitmap GetCover() {
		return coverBitmap;
	}

}
