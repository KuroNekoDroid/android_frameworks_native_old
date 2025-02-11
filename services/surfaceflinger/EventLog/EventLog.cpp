/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"

#include <stdio.h>
#include <stdlib.h>
#include <log/log.h>

#include "EventLog.h"

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(EventLog)


EventLog::EventLog() {
}

void EventLog::doLogFrameDurations(const std::string_view& name, const int32_t* durations,
                                   size_t numDurations) {
    EventLog::TagBuffer buffer(LOGTAG_SF_FRAME_DUR);
    buffer.startList(1 + numDurations);
    buffer.writeString(name);
    for (size_t i = 0; i < numDurations; i++) {
        buffer.writeInt32(durations[i]);
    }
    buffer.endList();
    buffer.log();
}

void EventLog::logFrameDurations(const std::string_view& name, const int32_t* durations,
                                 size_t numDurations) {
    EventLog::getInstance().doLogFrameDurations(name, durations, numDurations);
}

// ---------------------------------------------------------------------------

EventLog::TagBuffer::TagBuffer(int32_t tag)
    : mPos(0), mTag(tag), mOverflow(false) {
}

void EventLog::TagBuffer::log() {
    // purge the buffer
    mPos = 0;
    mOverflow = false;
}

void EventLog::TagBuffer::startList(int8_t count) {
    if (mOverflow) return;
    const size_t needed = 1 + sizeof(count);
    if (mPos + needed > STORAGE_MAX_SIZE) {
        mOverflow = true;
        return;
    }
    mStorage[mPos + 0] = EVENT_TYPE_LIST;
    mStorage[mPos + 1] = count;
    mPos += needed;
}

void EventLog::TagBuffer::endList() {
    if (mOverflow) return;
    const size_t needed = 1;
    if (mPos + needed > STORAGE_MAX_SIZE) {
        mOverflow = true;
        return;
    }
    mStorage[mPos + 0] = '\n';
    mPos += needed;
}

void EventLog::TagBuffer::writeInt32(int32_t value) {
    if (mOverflow) return;
    const size_t needed = 1 + sizeof(value);
    if (mPos + needed > STORAGE_MAX_SIZE) {
        mOverflow = true;
        return;
    }
    mStorage[mPos + 0] = EVENT_TYPE_INT;
    memcpy(&mStorage[mPos + 1], &value, sizeof(value));
    mPos += needed;
}

void EventLog::TagBuffer::writeInt64(int64_t value) {
    if (mOverflow) return;
    const size_t needed = 1 + sizeof(value);
    if (mPos + needed > STORAGE_MAX_SIZE) {
        mOverflow = true;
        return;
    }
    mStorage[mPos + 0] = EVENT_TYPE_LONG;
    memcpy(&mStorage[mPos + 1], &value, sizeof(value));
    mPos += needed;
}

void EventLog::TagBuffer::writeString(const std::string_view& value) {
    if (mOverflow) return;
    const size_t stringLen = value.length();
    const size_t needed = 1 + sizeof(int32_t) + stringLen;
    if (mPos + needed > STORAGE_MAX_SIZE) {
        mOverflow = true;
        return;
    }
    mStorage[mPos + 0] = EVENT_TYPE_STRING;
    memcpy(&mStorage[mPos + 1], &stringLen, sizeof(int32_t));
    memcpy(&mStorage[mPos + 5], value.data(), stringLen);
    mPos += needed;
}

} // namespace android

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic pop // ignored "-Wconversion"
