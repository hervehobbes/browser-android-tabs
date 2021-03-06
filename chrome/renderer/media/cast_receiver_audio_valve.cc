// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/media/cast_receiver_audio_valve.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "media/base/audio_parameters.h"

CastReceiverAudioValve::CastReceiverAudioValve(
    const media::AudioParameters& params,
    media::AudioCapturerSource::CaptureCallback* cb)
    : cb_(cb),
      fifo_(base::Bind(&CastReceiverAudioValve::DeliverRebufferedAudio,
                       base::Unretained(this))),
      sample_rate_(params.sample_rate()) {
  fifo_.Reset(params.frames_per_buffer());
}

CastReceiverAudioValve::~CastReceiverAudioValve() {}

void CastReceiverAudioValve::DeliverDecodedAudio(
    const media::AudioBus* audio_bus,
    base::TimeTicks playout_time) {
  current_playout_time_ = playout_time;
  // The following will result in zero, one, or multiple synchronous calls to
  // DeliverRebufferedAudio().
  fifo_.Push(*audio_bus);
}

void CastReceiverAudioValve::DeliverRebufferedAudio(
    const media::AudioBus& audio_bus,
    int frame_delay) {
  const base::TimeTicks playout_time =
      current_playout_time_ +
      base::TimeDelta::FromMicroseconds(
          frame_delay * base::Time::kMicrosecondsPerSecond / sample_rate_);

  base::AutoLock lock(lock_);
  if (cb_) {
    // The AudioCapturerSource::Callback interface provides timing information
    // only as a relative delay value that means "captured N milliseconds ago."
    // Therefore, translate the playout time to these semantics.  Since playout
    // time is generally in the future, because audio should be decoded well
    // ahead of when it should be played out, the audio delay value will
    // generally be negative.
    //
    // TimeTicks::Now() is being called after the |lock_| was acquired in order
    // to provide the most accurate delay value.
    const int audio_delay_milliseconds =
        (base::TimeTicks::Now() - playout_time).InMilliseconds();

    cb_->Capture(&audio_bus, audio_delay_milliseconds, 1.0 /* volume */,
                 false /* key_pressed */);
  }
}

void CastReceiverAudioValve::Stop() {
  base::AutoLock lock(lock_);
  cb_ = nullptr;
}
