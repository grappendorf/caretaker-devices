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


import java.io.IOException;
import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;
import android.app.Activity;
import android.appwidget.AppWidgetManager;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.*;
import android.view.View.OnClickListener;
import android.widget.*;
import android.widget.SeekBar.OnSeekBarChangeListener;


public class DimmerController extends Activity
{
	private static final String TAG = "DimmerController";

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.dimmer_controller);

		Bundle extras = getIntent().getExtras();
		final int appWidgetId = extras != null ? extras.getInt("appWidgetId", AppWidgetManager.INVALID_APPWIDGET_ID)
						: AppWidgetManager.INVALID_APPWIDGET_ID;
		if (appWidgetId == AppWidgetManager.INVALID_APPWIDGET_ID)
		{
			finish();
			return;
		}

		findViewById(R.id.buttonBack).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				finish();
			}
		});

		findViewById(R.id.dimmer_controller).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				finish();
			}
		});

		((SeekBar) findViewById(R.id.value)).setOnSeekBarChangeListener(new OnSeekBarChangeListener()
		{

			@Override
			public void onStopTrackingTouch(SeekBar seekBar)
			{
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar)
			{
			}

			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
			{
				try
				{
					SharedPreferences prefs = getSharedPreferences(DimmerWidgetConfigurer.PREFS_NAME, 0);
					AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(DimmerController.this
									.getApplicationContext());

					String serverUrl = prefs.getString(DimmerWidgetConfigurer.PREFS_SERVER_URL, "");
					String deviceAddress = prefs.getString(DimmerWidgetConfigurer.PREFS_DEVICE_ADDRESS + appWidgetId,
									"");

					HttpClient client = new DefaultHttpClient();
					HttpGet httpGet = new HttpGet(serverUrl + "/api/device/" + deviceAddress + "/dimmer/"
									+ seekBar.getProgress());
					HttpResponse response = client.execute(httpGet);
					EntityUtils.toString(response.getEntity());

					httpGet = new HttpGet(serverUrl + "/api/device/" + deviceAddress + "/dimmer");
					response = client.execute(httpGet);
					Integer value = new Integer(EntityUtils.toString(response.getEntity()));
					((TextView) findViewById(R.id.valueText)).setText(String.valueOf(value));
					RemoteViews remoteViews = new RemoteViews(DimmerController.this.getApplicationContext()
									.getPackageName(), R.layout.dimmer_widget);
					remoteViews.setInt(R.id.dimmer_value, "setProgress", value);
					remoteViews.setInt(
									R.id.dimmer_widget_layout,
									"setBackgroundResource",
									value > 128 ? DimmerWidgetConfigurer.backgroundResources[prefs.getInt(
													DimmerWidgetConfigurer.PREFS_COLOR + appWidgetId, 0)]
													: R.drawable.widget_grey_shape);
					appWidgetManager.updateAppWidget(appWidgetId, remoteViews);
				}
				catch (ClientProtocolException x)
				{
					Log.e(TAG, x.getMessage());
				}
				catch (IOException x)
				{
					Log.e(TAG, x.getMessage());
				}
				catch (ParseException x)
				{
					Log.e(TAG, x.getMessage());
				}
			}
		});

		try
		{
			SharedPreferences prefs = getSharedPreferences(DimmerWidgetConfigurer.PREFS_NAME, 0);
			String serverUrl = prefs.getString(DimmerWidgetConfigurer.PREFS_SERVER_URL, "");
			String deviceAddress = prefs.getString(DimmerWidgetConfigurer.PREFS_DEVICE_ADDRESS + appWidgetId, "");

			HttpClient client = new DefaultHttpClient();
			HttpGet httpGet = new HttpGet(serverUrl + "/api/device/" + deviceAddress + "/dimmer");
			HttpResponse response = client.execute(httpGet);
			Integer value = new Integer(EntityUtils.toString(response.getEntity()));
			((SeekBar) findViewById(R.id.value)).setProgress(value);
			((TextView) findViewById(R.id.valueText)).setText(String.valueOf(value));
		}
		catch (ClientProtocolException x)
		{
			Log.e(TAG, x.getMessage());
		}
		catch (IOException x)
		{
			Log.e(TAG, x.getMessage());
		}
		catch (ParseException x)
		{
			Log.e(TAG, x.getMessage());
		}
	}
}
