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


import android.app.PendingIntent;
import android.appwidget.*;
import android.content.*;
import android.util.Log;
import android.widget.RemoteViews;


public class DimmerWidgetProvider extends AppWidgetProvider
{
	private static final String TAG = "DimmerWidgetProvider";

	@Override
	public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds)
	{
		Log.w(TAG, "onUpdate Dimmer *******");
		for (int appWidgetId : appWidgetIds)
		{
			Log.w(TAG, "onUpdate Dimmer " + appWidgetId);

			updateAppWidget(context, appWidgetManager, appWidgetId);
		}
	}

	public static void updateAppWidget(Context context, AppWidgetManager appWidgetManager, int appWidgetId)
	{
		RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.dimmer_widget);
		SharedPreferences prefs = context.getSharedPreferences(DimmerWidgetConfigurer.PREFS_NAME, 0);
		views.setTextViewText(R.id.dimmer_widget_label, prefs.getString(DimmerWidgetConfigurer.PREFS_LABEL
						+ appWidgetId, context.getResources().getString(R.string.dimmer_label)));

		// Intent intent = new Intent(context, DimmerWidgetService.class);
		Intent intent = new Intent(context, DimmerController.class);
		intent.setAction("click-" + appWidgetId);
		intent.putExtra("appWidgetId", appWidgetId);
		// PendingIntent pendingIntent = PendingIntent.getService(context, 0,
		// intent, 0);
		// views.setOnClickPendingIntent(R.id.dimmer_widget_layout,
		// pendingIntent);
		PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, intent, 0);
		views.setOnClickPendingIntent(R.id.dimmer_widget_layout, pendingIntent);

		intent = new Intent(context, DimmerWidgetConfigurer.class);
		intent.setAction("configure-" + appWidgetId);
		intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, appWidgetId);
		pendingIntent = PendingIntent.getActivity(context, 0, intent, 0);
		views.setOnClickPendingIntent(R.id.dimmer_widget_configure, pendingIntent);

		appWidgetManager.updateAppWidget(appWidgetId, views);
	}
}
