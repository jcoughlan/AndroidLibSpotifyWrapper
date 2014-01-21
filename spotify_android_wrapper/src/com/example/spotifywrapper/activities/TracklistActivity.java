package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import android.app.ListActivity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;

import com.example.classes.Instance;
import com.example.classes.Track;
import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;

public class TracklistActivity extends ListActivity {
	private ArrayList<String> listItems = new ArrayList<String>();
	private ServiceBinder binder;
	private ArrayAdapter<String> adapter;
	private ListView tracklistListView = null;

	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
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
				// TODO Auto-generated method stub
				Track track = Instance.currentInstance.GetCurrentPlaylist()
						.GetTrackList().GetTrack(arg2);
				Instance.currentInstance.SetCurrentTrack(track);
				Intent playerIntent = new Intent(TracklistActivity.this,
						PlayerActivity.class);
				startActivity(playerIntent);
			}
		});
	}

	private void populateTrackList() {
		// TODO Auto-generated method stub
		for (int i = 0; i < Instance.currentInstance.GetCurrentPlaylist()
				.Length(); i++) {
			listItems.add((String) Instance.currentInstance
					.GetCurrentPlaylist().GetTrackList().GetTrack(i)
					.getTrackName());
			adapter.notifyDataSetChanged();
		}
		Bitmap cover = Instance.currentInstance.GetCurrentPlaylist().GetCover();
		if (cover != null)
			((ImageView) findViewById(R.id.coverImage)).setImageBitmap(cover);
	}

}
