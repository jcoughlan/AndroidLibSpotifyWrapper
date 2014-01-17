package com.example.spotifywrapper.activities;

import java.util.ArrayList;

import com.example.spotifywrapper.R;
import com.example.spotifywrapper.ServiceBinder;
import com.example.spotifywrapper.ServiceBinder.ServiceBinderDelegate;
import com.example.spotifywrapper.SpotifyService.PlaylistNamesDelegate;

import android.app.ListActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AdapterView.OnItemClickListener;

public class TracklistActivity  extends ListActivity {
	private ArrayList<String> listItems = new ArrayList<String>();
	private ServiceBinder binder;
	private ArrayAdapter<String> adapter;
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_tracklist);
		
		binder = new ServiceBinder(this);
		
		
		adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_1, listItems);
		setListAdapter(adapter);
		
		

	}

}
