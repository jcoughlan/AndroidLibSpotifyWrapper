package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.annotation.SuppressLint;
import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.example.classes.Playlist;
import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.PlaylistNamesDelegate;

public class PlaylistActivity extends ListActivity {
	private int clickCounter = 0;
	private ArrayAdapter<String> adapter;
	private ArrayList<String> listItems = new ArrayList<String>();
	private ServiceBinder binder;
	private ArrayList<Playlist> playlistList = new ArrayList<Playlist>();
	private ListView playlistListView = null;

	@SuppressLint("NewApi")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_playlist);
		playlistListView = this.getListView();
		binder = new ServiceBinder(this);

		binder.bindService(new ServiceBinderDelegate() {

			@Override
			public void onIsBound() {
				binder.getService().fetchAllPlaylistNames(
						new PlaylistNamesDelegate() {

							@Override
							public void onPlaylistNameFetched(String name) {
								addPlaylist(name);
							}

							@Override
							public void onTrackFetched(String name,
									String playlistName) {								
								for (int i = 0; i < playlistList.size(); i++) {
									if (playlistList.get(i).GetTitle()
											.equals(playlistName)) {
										playlistList.get(i).AddTrack(name);
										Log.i("Added", "" + name + " to "
												+ playlistName);
										return;
									}
								}
								//if we get here we havent found the tracks playlist
							}

						});
			}
		});
		adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_1, listItems);
		setListAdapter(adapter);

		playlistListView.setOnItemClickListener(new OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				Intent tracklistIntent = new Intent(PlaylistActivity.this,
						TracklistActivity.class);
				startActivity(tracklistIntent);

			}
		});

	}

	public void addPlaylist(String playlistName) {
		Playlist playlist = new Playlist();
		playlistList.add(playlist);
		addItems("* " + playlistName);
	}

	public void addItems(String text) {
		listItems.add(text);
		adapter.notifyDataSetChanged();
	}

}
