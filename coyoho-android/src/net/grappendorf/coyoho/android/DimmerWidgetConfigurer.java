/*
 * This file is part of the CoYoHo Control Your Home project.
 *
 * Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package net.grappendorf.coyoho.android;


import android.app.Activity;
import android.appwidget.AppWidgetManager;
import android.content.*;
import android.os.Bundle;
import android.util.Log;
import android.view.*;
import android.view.View.OnClickListener;
import android.widget.*;


public class DimmerWidgetConfigurer extends Activity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		Bundle extras = getIntent().getExtras();
		final int appWidgetId = extras != null ? extras.getInt(AppWidgetManager.EXTRA_APPWIDGET_ID,
						AppWidgetManager.INVALID_APPWIDGET_ID) : AppWidgetManager.INVALID_APPWIDGET_ID;
		if (appWidgetId == AppWidgetManager.INVALID_APPWIDGET_ID)
		{
			Log.e(TAG, "No app widget id specified");
			finish();
		}

		Intent cancelResultIntent = new Intent();
		cancelResultIntent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, appWidgetId);
		setResult(RESULT_CANCELED, cancelResultIntent);

		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.dimmer_widget_configurer);

		SharedPreferences prefs = getSharedPreferences(PREFS_NAME, 0);
		((EditText) findViewById(R.id.serverUrl)).setText(prefs.getString(PREFS_SERVER_URL, ""));
		((EditText) findViewById(R.id.deviceAddress)).setText(prefs.getString(PREFS_DEVICE_ADDRESS + appWidgetId, ""));
		((EditText) findViewById(R.id.label)).setText(prefs.getString(PREFS_LABEL + appWidgetId, getResources()
						.getString(R.string.dimmer_label)));
		((Spinner) findViewById(R.id.color)).setSelection(prefs.getInt(PREFS_COLOR + appWidgetId, 0));

		findViewById(R.id.buttonSave).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				SharedPreferences.Editor prefs = getSharedPreferences(PREFS_NAME, 0).edit();
				prefs.putString(PREFS_SERVER_URL, ((EditText) findViewById(R.id.serverUrl)).getText().toString());
				prefs.putString(PREFS_DEVICE_ADDRESS + appWidgetId, ((EditText) findViewById(R.id.deviceAddress))
								.getText().toString());
				prefs.putString(PREFS_LABEL + appWidgetId, ((EditText) findViewById(R.id.label)).getText().toString());
				prefs.putInt(PREFS_COLOR + appWidgetId, ((Spinner) findViewById(R.id.color)).getSelectedItemPosition());
				prefs.commit();

				AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(DimmerWidgetConfigurer.this);
				DimmerWidgetProvider.updateAppWidget(DimmerWidgetConfigurer.this, appWidgetManager, appWidgetId);

				Intent resultValue = new Intent();
				resultValue.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, appWidgetId);
				setResult(RESULT_OK, resultValue);
				finish();
			}
		});

		findViewById(R.id.buttonCancel).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				finish();
			}
		});
	}

	private static final String TAG = "DimmerWidgetConfigurer";

	public static final String PREFS_NAME = "net.grappendorf.coyoho.android.Preferences";

	public static final String PREFS_SERVER_URL = "serverUrl";

	public static final String PREFS_DEVICE_ADDRESS = "deviceAddress_";

	public static final String PREFS_LABEL = "label_";

	public static final String PREFS_COLOR = "color_";

	public static int[] backgroundResources =
	{
		R.drawable.widget_red_shape, R.drawable.widget_green_shape, R.drawable.widget_yellow_shape
	};
}
