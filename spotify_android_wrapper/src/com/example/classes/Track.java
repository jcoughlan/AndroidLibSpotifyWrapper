

package com.example.classes;

public class Track {
	private String mTrack;
	private String mAlbum;
	private String mArtist;
	private String mUri;

	public Track(String track, String album, String artist, String uri) {
		mTrack = track;
		mAlbum = album;
		mArtist = artist;
		mUri = uri;
	}

	public String getSpotifyUri() {
		return mUri;
	}

	public String getTrackInfo() {
		return mAlbum + " - " + mArtist;
	}

	public CharSequence getTrackName() {
		return mTrack;
	}

}