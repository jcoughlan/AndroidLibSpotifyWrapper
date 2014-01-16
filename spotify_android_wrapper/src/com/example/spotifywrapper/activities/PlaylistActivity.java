package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.annotation.SuppressLint;
import android.app.ListActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ArrayAdapter;

import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.PlaylistContainerDelegate;

public class PlaylistActivity extends ListActivity {
	private int clickCounter = 0;
	private ArrayAdapter<String> adapter;
	private ArrayList<String> listItems = new ArrayList<String>();
	private ServiceBinder binder;

	@SuppressLint("NewApi")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_playlist);
		binder = new ServiceBinder(this);
		binder.bindService(new ServiceBinderDelegate() {

			@Override
			public void onIsBound() {
				binder.getService().fetchAllPlaylistContainers(new PlaylistContainerDelegate() {
					
					@Override
					public void onPlaylistContainerFetchSuccess() {
						// TODO Auto-generated method stub
						Log.i("PLAYLIST FETCH", "SUCCESS");
						
					}
					
					@Override
					public void onPlaylistContainerFetchFailed() {
						// TODO Auto-generated method stub
						
					}
				});  
			}
		});
		adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_1, listItems);
		setListAdapter(adapter);
		

	}

	public void addItems(String text) {
		listItems.add("Clicked : " + clickCounter++);
		adapter.notifyDataSetChanged();
	}

}
