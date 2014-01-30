package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.annotation.SuppressLint;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SearchView.OnQueryTextListener;

import com.example.classes.AppInstance;
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
	private ProgressDialog pdialog = null;
	private SearchView searchView = null;

	@SuppressLint("NewApi")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_playlist);
		playlistListView = this.getListView();
		searchView = (SearchView) findViewById(R.id.searchView);
		pdialog = new ProgressDialog(this);
		pdialog.setCancelable(false);
		pdialog.setMessage("Loading ....");
		// pdialog.show();
		
		searchView.setSubmitButtonEnabled(true);
		searchView.setOnQueryTextListener(new OnQueryTextListener() {
			
			@Override
			public boolean onQueryTextSubmit(String query) {
				
				Intent searchIntent = new Intent(PlaylistActivity.this,
						SearchResultActivity.class);
				searchIntent.putExtra("query", query);
				startActivity(searchIntent);
				return false;
			}
			
			@Override
			public boolean onQueryTextChange(String newText) {
				// TODO Auto-generated method stub
				return false;
			}
		});
		binder = new ServiceBinder(this);

	binder.bindService(new ServiceBinderDelegate() {

			@Override
			public void onIsBound() {
				binder.getService().fetchAllPlaylistsAndTracks(
						new AllPlaylistsAndTracksDelegate() {

							@Override
							public void onPlaylistFetched(String name,
									byte[] imageBytes) {
								// Add a playlist
								byte[] data = imageBytes;
								Bitmap bmp;
								bmp = BitmapFactory.decodeByteArray(data, 0,
										data.length);
								addPlaylist(name, bmp);
								Drawable d = new BitmapDrawable(getResources(),
										bmp);
								pdialog.setIcon(d);
							}

							@Override
							public void onTrackFetched(String name,
									String playlistName, String artistName,
									String albumName, String uri) {

								// send track to corresponding playlist and
								// populate tracklist
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
								// (should never reach here!)
								Log.i("TRACK", "NULL OR NO PLAYLIST");
							}

							@Override
							public void onAllPlaylistsAndTracksLoaded() {
								// cancel progress dialog
								if (pdialog != null)
									pdialog.cancel();
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
				// when clicked, set as current playlist and go to tracklist
				// view
				AppInstance.currentInstance.SetCurrentPlaylist(playlistList
						.get(arg2));
				Intent tracklistIntent = new Intent(PlaylistActivity.this,
						TracklistActivity.class);
				startActivity(tracklistIntent);
			}
		});
	}

	public void addPlaylist(String playlistName, Bitmap bmp) {
		// set title and cover of playlist
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
