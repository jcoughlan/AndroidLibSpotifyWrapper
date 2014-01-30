package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.ExpandableListView;

import com.example.classes.Album;
import com.example.classes.Playlist;
import com.example.classes.Track;
import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.SearchDelegate;

public class SearchResultActivity extends Activity {
	private ArrayList<Playlist> playlistList = new ArrayList<Playlist>();
	private ArrayList<Album> albumList = new ArrayList<Album>();
	private ArrayList<Track> trackList = new ArrayList<Track>();
	private ExpandableListView trackListView = null;
	private ExpandableListView playlistListView = null;
	private ExpandableListView albumListView = null;
	private String query = null;
	private ServiceBinder binder;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_search_result);
		Intent i = getIntent();
		query = i.getStringExtra("query");

		trackListView = (ExpandableListView) findViewById(R.id.trackSearchResult);
		playlistListView = (ExpandableListView) findViewById(R.id.playlistSearchResult);
		albumListView = (ExpandableListView) findViewById(R.id.albumSearchResult);
		binder = new ServiceBinder(this);

		binder.bindService(new ServiceBinderDelegate() {

			@Override
			public void onIsBound() {
				binder.getService().fetchSearchResults(query, 10, 10, 15, 5,
						new SearchDelegate() {

							@Override
							public void onTrackSearchReceived(String trackName) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onPlaylistSearchReceived(
									String playlistName) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onAlbumSearchReceived(String albumName) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onArtistSearchReceived(String artistName) {
								// TODO Auto-generated method stub

							}
						});

			}
		});
	}
}
