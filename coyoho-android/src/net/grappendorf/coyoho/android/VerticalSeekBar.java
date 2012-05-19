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


import android.content.Context;
import android.graphics.Canvas;
import android.util.*;
import android.view.MotionEvent;
import android.widget.SeekBar;


public class VerticalSeekBar extends SeekBar
{
	private OnSeekBarChangeListener _seekbarListener;

	public VerticalSeekBar(Context context)
	{
		super(context);
	}

	public VerticalSeekBar(Context context, AttributeSet attrs, int defStyle)
	{
		super(context, attrs, defStyle);
	}

	public VerticalSeekBar(Context context, AttributeSet attrs)
	{
		super(context, attrs);
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh)
	{
		super.onSizeChanged(h, w, oldh, oldw);
	}

	@Override
	protected synchronized void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
	{
		super.onMeasure(heightMeasureSpec, widthMeasureSpec);
		setMeasuredDimension(getMeasuredHeight(), getMeasuredWidth());
	}

	@Override
	public void setOnSeekBarChangeListener(OnSeekBarChangeListener mListener)
	{
		this._seekbarListener = mListener;
	}

	@Override
	protected void onDraw(Canvas c)
	{

		c.rotate(- 90);
		c.translate(- getHeight(), 0);

		super.onDraw(c);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		if (! isEnabled() || _seekbarListener == null)
		{
			return false;
		}

		switch (event.getAction())
		{

			case MotionEvent.ACTION_DOWN:
				if (_seekbarListener != null)
					_seekbarListener.onStartTrackingTouch(this);
				break;

			case MotionEvent.ACTION_MOVE:
				int position = getMax() - (int) (getMax() * event.getY() / getHeight());

				if (position < 0)
					position = 0;
				if (position > getMax())
					position = getMax();

				setProgress(position);
				onSizeChanged(getWidth(), getHeight(), 0, 0);
				if (_seekbarListener != null)
					_seekbarListener.onProgressChanged(this, position, true);
				break;

			case MotionEvent.ACTION_UP:
				if (_seekbarListener != null)
					_seekbarListener.onStopTrackingTouch(this);
				break;

			case MotionEvent.ACTION_CANCEL:
				break;

		}

		return true;

	}

}
