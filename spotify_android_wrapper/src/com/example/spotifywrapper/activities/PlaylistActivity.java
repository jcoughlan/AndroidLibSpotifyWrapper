package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.annotation.SuppressLint;
import android.app.ListActivity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.example.classes.Instance;
import com.example.classes.Playlist;
import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.AllPlaylistsAndTracksDelegate;

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
				binder.getService().fetchAllPlaylistsAndTracks(
						new AllPlaylistsAndTracksDelegate() {

							@Override
							public void onPlaylistFetched(String name, byte[] imageBytes) {
								
								Log.i("TAG", "Imagebytes length: "+ imageBytes.length);
								byte[] data = imageBytes;
								Bitmap bmp;
								bmp = BitmapFactory.decodeByteArray(data, 0, data.length);
								addPlaylist(name, bmp);
							}
							@Override
							public void onTrackFetched(String name,
									String playlistName, String artistName,
									String albumName, String uri) {
								//Log.i("In", "Callback " + playlistName);
								if (name != null) {
									for (int i = 0; i < playlistList.size(); i++) {
										if (playlistList.get(i).GetTitle()
												.equals(playlistName)) {
											playlistList.get(i).AddTrack(name,
													albumName, artistName, uri);
											return;
										}
									}
								}
								// if we get here we havent found the tracks
								// playlist
								Log.i("TRACK", "NULL OR NO PLAYLIST");
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
				Instance.currentInstance.SetCurrentPlaylist( playlistList.get(arg2));
				Intent tracklistIntent = new Intent(PlaylistActivity.this,
						TracklistActivity.class);
				startActivity(tracklistIntent);

			}
		});

	}

	public void addPlaylist(String playlistName, Bitmap bmp) {
		Playlist playlist = new Playlist();
		playlistList.add(playlist);
		playlistList.get(playlistList.size() - 1).SetTitle(playlistName);
		playlistList.get(playlistList.size() - 1).SetCover(bmp);
		addItems("* " + playlistName);
	}

	public void addItems(String text) {
		listItems.add(text);
		adapter.notifyDataSetChanged();
	}

}
