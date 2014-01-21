package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;

import com.example.classes.AppInstance;
import com.example.classes.Track;
import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.AlbumInfoDelegate;

public class TracklistActivity extends ListActivity {
	private ArrayList<String> listItems = new ArrayList<String>();
	private ServiceBinder binder;
	private ArrayAdapter<String> adapter;
	private ListView tracklistListView = null;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_tracklist);

		binder = new ServiceBinder(this);

		adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_1, listItems);
		setListAdapter(adapter);

		populateTrackList();

		tracklistListView = this.getListView();
		tracklistListView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// set chosen track to the current track & open the player
				Track track = AppInstance.currentInstance.GetCurrentPlaylist()
						.GetTrackList().GetTrack(arg2);
				AppInstance.currentInstance.SetCurrentTrack(track);
				Intent playerIntent = new Intent(TracklistActivity.this,
						PlayerActivity.class);
				startActivity(playerIntent);
			}
		});
	}

	private void populateTrackList() {
		// add the title of each track to the list
		for (int i = 0; i < AppInstance.currentInstance.GetCurrentPlaylist()
				.Length(); i++) {
			listItems.add((String) AppInstance.currentInstance
					.GetCurrentPlaylist().GetTrackList().GetTrack(i)
					.getTrackName());
			adapter.notifyDataSetChanged();
		}

		// set the cover
		Bitmap cover =AppInstance.currentInstance.GetCurrentPlaylist().GetCover();
		if (cover != null)
			((ImageView) findViewById(R.id.coverImage)).setImageBitmap(cover);
		else {
			// (if it doesnt have a playlist cover)
			// I have no idea if this is right but just use the first tracks
			// cover
			binder = new ServiceBinder(this);

			binder.bindService(new ServiceBinderDelegate() {

				@Override
				public void onIsBound() {
					binder.getService()
							.fetchAlbumInfo(
									AppInstance.currentInstance
											.GetCurrentPlaylist()
											.GetTrackList().GetTrack(0)
											.getSpotifyUri(),
									new AlbumInfoDelegate() {

										@Override
										public void onImageBytesReceived(
												byte[] bytes) {
											byte[] data = bytes;
											Bitmap bmp;
											bmp = BitmapFactory
													.decodeByteArray(data, 0,
															data.length);
											((ImageView) findViewById(R.id.coverImage))
													.setImageBitmap(bmp);
										}
									});
				}
			});
		}
	}
}
