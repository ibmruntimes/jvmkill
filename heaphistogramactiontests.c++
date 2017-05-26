/*
 * Copyright (c) 2015 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "heaphistogramaction.h"
#include "heapstats.h"

HeapHistogramAction* heapHistogramAction;

JNIEnv* mockJNIEnv;

struct JVMTINativeInterface_ mockJvmtiInterface_1_;
jvmtiEnv mockJvmtiEnvStruct;
jvmtiEnv *mockJvmtiEnv;

static int MockGetCapabilitiesCount;
static jvmtiError MockGetCapabilitiesReturnValue;
jvmtiError (JNICALL MockGetCapabilities) (jvmtiEnv* env,
	jvmtiCapabilities* capabilities) {
	MockGetCapabilitiesCount++;
	capabilities->can_generate_garbage_collection_events = 0;
	return MockGetCapabilitiesReturnValue;
}

static int MockAddCapabilitiesCount;
static jvmtiError MockAddCapabilitiesReturnValue;
static jvmtiCapabilities MockAddedCapabilities;
jvmtiError (JNICALL MockAddCapabilities) (jvmtiEnv* env,
	const jvmtiCapabilities* capabilities) {
	MockAddCapabilitiesCount++;
	memcpy(&MockAddedCapabilities, capabilities, sizeof(jvmtiCapabilities));
	return MockAddCapabilitiesReturnValue;
}

static int MockGetLoadedClassesCount;
static jint MockGetLoadedClassesResultantClassCount;
static jclass* MockGetLoadedClassesResultantClasses;
static jvmtiError MockGetLoadedClassesReturnValue;
jvmtiError (JNICALL MockGetLoadedClasses) (jvmtiEnv* env,
    jint* classCount,
    jclass** classes) {
	MockGetLoadedClassesCount++;
	*classCount = MockGetLoadedClassesResultantClassCount;
	*classes = MockGetLoadedClassesResultantClasses;
	return MockGetLoadedClassesReturnValue;
}

static int MockFollowReferencesCount;
static jvmtiHeapReferenceCallback MockHeapReferenceCallback;
static jint (*MockFollowReferencesAction)();
static jvmtiError MockFollowReferencesReturnValue;
jvmtiError (JNICALL MockFollowReferences) (jvmtiEnv* env,
  jint heap_filter,
  jclass klass,
  jobject initial_object,
  const jvmtiHeapCallbacks* callbacks,
  const void* user_data) {
	MockFollowReferencesCount++;
	MockHeapReferenceCallback = callbacks->heap_reference_callback;
	if (MockFollowReferencesAction != NULL) {
		(*MockFollowReferencesAction)();
	}

  return MockFollowReferencesReturnValue;
}

static int MockGetClassSignatureCount;
static char* MockGetClassSignatureResult;
static jvmtiError MockGetClassSignatureReturnValue;
jvmtiError (JNICALL MockGetClassSignature) (jvmtiEnv* env,
	jclass klass,
	char** signature_ptr,
	char** generic_ptr) {
	MockGetClassSignatureCount++;
	if (MockGetClassSignatureResult != NULL) {
		*signature_ptr = MockGetClassSignatureResult;		
	}
	return MockGetClassSignatureReturnValue;
}

static vector<jlong> MockSetTagTagsSet;
static jvmtiError MockSetTagReturnValue;
jvmtiError (JNICALL MockSetTag) (jvmtiEnv* env,
	jobject object,
	jlong tag) {
	MockSetTagTagsSet.push_back(tag);
	return MockSetTagReturnValue;
}

static int MockDeallocateCount;
static jvmtiError MockDeallocateReturnValue;
jvmtiError (JNICALL MockDeallocate) (jvmtiEnv* env,
	unsigned char* mem) {
	MockDeallocateCount++;
	return MockDeallocateReturnValue;
}

static int MockRecordObjectCount;
static const char* MockRecordObjectClassName;
static size_t MockRecordObjectSize;
static int MockPrintCount;
const jlong VISITED_OBJECT = 1 << 31;

class MockHeapStats: public HeapStats {
public:
   MockHeapStats() {}
   virtual ~MockHeapStats() {}

   void recordObject(const char *className, size_t objectSize) {
   	MockRecordObjectCount++;
     MockRecordObjectClassName = className;
     MockRecordObjectSize = objectSize;
   }

   void print(std::ostream& os) const {
   	 MockPrintCount++;
   }
};

class MockHeapStatsFactory: public HeapStatsFactory {
public:
   MockHeapStatsFactory() {}

   virtual ~MockHeapStatsFactory() {}

   HeapStats* create() {
     return new MockHeapStats();
   }
};

static MockHeapStatsFactory* MockHSFactory;

void setup() {
	heapHistogramAction = NULL;

	mockJNIEnv = 0;

	mockJvmtiEnvStruct.functions = &mockJvmtiInterface_1_;

	MockGetCapabilitiesCount = 0;
	MockGetCapabilitiesReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->GetCapabilities = &MockGetCapabilities;

	MockAddCapabilitiesCount = 0;
	MockAddCapabilitiesReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->AddCapabilities = &MockAddCapabilities;

	MockGetLoadedClassesCount = 0;
	MockGetLoadedClassesReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->GetLoadedClasses = &MockGetLoadedClasses;

	MockSetTagReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->SetTag = &MockSetTag;

	MockGetClassSignatureCount = 0;
	MockGetClassSignatureResult = NULL;
	MockGetClassSignatureReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->GetClassSignature = &MockGetClassSignature;

	MockDeallocateCount = 0;
	MockDeallocateReturnValue = JVMTI_ERROR_NONE;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->Deallocate = &MockDeallocate;

	MockFollowReferencesCount = 0;
	MockFollowReferencesReturnValue = JVMTI_ERROR_NONE;
	MockFollowReferencesAction = NULL;
	((struct JVMTINativeInterface_ *)mockJvmtiEnvStruct.functions)->FollowReferences = &MockFollowReferences;

	mockJvmtiEnv = &mockJvmtiEnvStruct;

	MockHSFactory = new MockHeapStatsFactory();
	MockRecordObjectCount = 0;
	MockPrintCount = 0;
}

void teardown() {
	if (heapHistogramAction != NULL) {		
		delete heapHistogramAction;
	}
}

bool testConstructionOk() {
	setup();

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

	bool passed = (MockGetCapabilitiesCount == 1) &&
					(MockAddCapabilitiesCount == 1) &&
					(MockAddedCapabilities.can_tag_objects == 1);

	if (!passed) {
		fprintf(stdout, "testConstruction FAILED\n");
	}

	teardown();
	return passed;
}

bool testConstructionGetCapabilitiesFailure() {
	setup();
	MockGetCapabilitiesReturnValue = JVMTI_ERROR_ACCESS_DENIED;

    bool passed = false;
    try {
		heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);
    } catch (std::runtime_error *re) {
    	passed = (MockGetCapabilitiesCount == 1) &&
    				(MockAddCapabilitiesCount == 0);
	}

	if (!passed) {
		fprintf(stdout, "testConstructionGetCapabilitiesFailure FAILED\n");
	}

	teardown();
	return passed;
}

bool testConstructionAddCapabilitiesFailure() {
	setup();
	MockAddCapabilitiesReturnValue = JVMTI_ERROR_ACCESS_DENIED;

    bool passed = false;
    try {
		heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);
    } catch (std::runtime_error *re) {
    	passed = (MockGetCapabilitiesCount == 1) &&
    				(MockAddCapabilitiesCount == 1);
	}

	if (!passed) {
		fprintf(stdout, "testConstructionAddCapabilitiesFailure FAILED\n");
	}

	teardown();
	return passed;
}

bool testHeapStatsPrintOk() {
	setup();
	MockGetLoadedClassesResultantClassCount = 0;

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);
	heapHistogramAction->act(mockJNIEnv, 0);
	bool passed = (MockGetLoadedClassesCount == 1) &&
					(MockPrintCount == 1);

	if (!passed) {
		fprintf(stdout, "testHeapStatsPrintOk FAILED\n");
	}

	teardown();
	return passed;
}

bool testHeapStatsGetLoadedClassesFailure() {
	setup();
	MockGetLoadedClassesReturnValue = JVMTI_ERROR_ACCESS_DENIED;

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

    bool passed = false;
    try {
		heapHistogramAction->act(mockJNIEnv, 0);
    } catch (std::runtime_error *re) {
    	passed = (MockGetCapabilitiesCount == 1) &&
    				(MockAddCapabilitiesCount == 1) &&
    				(MockGetLoadedClassesCount == 1);
	}

	if (!passed) {
		fprintf(stdout, "testHeapStatsGetLoadedClassesFailure FAILED\n");
	}

	teardown();
	return passed;
}

bool testFollowReferencesFailure() {
	setup();
	MockGetLoadedClassesResultantClassCount = 0;
	MockFollowReferencesReturnValue = JVMTI_ERROR_ACCESS_DENIED;

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

    bool passed = false;
	try {
		heapHistogramAction->act(mockJNIEnv, 0);
    } catch (std::runtime_error *re) {
    	passed = (MockFollowReferencesCount == 1);
	}

	if (!passed) {
		fprintf(stdout, "testFollowReferencesFailure FAILED\n");
	}

	teardown();
	return passed;
}

bool testHeapStatsTagsClassesReturnedByGetLoadedClasses() {
	setup();
	MockGetLoadedClassesResultantClassCount = 2;
	MockGetLoadedClassesResultantClasses = new jclass[2];
	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

	heapHistogramAction->act(mockJNIEnv, 0);
	bool passed = ((MockSetTagTagsSet[0] == 1) &&
						(MockSetTagTagsSet[1] == 2));

	if (!passed) {
		fprintf(stdout, "tags %lld %lld\n", MockSetTagTagsSet[0], MockSetTagTagsSet[1]);
		fprintf(stdout, "testHeapStatsTagsClassesReturnedByGetLoadedClasses FAILED\n");
	}

	teardown();
	return passed;
}

bool testHeapRefCallback() {
	setup();

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);
	heapHistogramAction->act(mockJNIEnv, 0);

	jlong tag = 0;
	jint heapVisitControlFlags = (*MockHeapReferenceCallback)(JVMTI_HEAP_REFERENCE_CLASS,
																NULL,
																0,
																0,
																0,
																&tag,
																NULL,
																0,
																heapHistogramAction);
	bool passed = heapVisitControlFlags == JVMTI_VISIT_OBJECTS;

	if (!passed) {
		fprintf(stdout, "heapVisitControlFlags, expected %d, found %d\n", JVMTI_VISIT_OBJECTS, heapVisitControlFlags);
		fprintf(stdout, "testHeapRefCallback FAILED\n");
	}

	teardown();
	return passed;
}

bool testHeapRefCallbackNoVisit() {
	setup();

	MockGetLoadedClassesResultantClassCount = 0;

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);
	heapHistogramAction->act(mockJNIEnv, 0);

	jlong tag = VISITED_OBJECT;
	jint heapVisitControlFlags = (*MockHeapReferenceCallback)(JVMTI_HEAP_REFERENCE_CLASS,
																NULL,
																0,
																0,
																0,
																&tag,
																NULL,
																0,
																heapHistogramAction);
	bool passed = heapVisitControlFlags == 0;

	if (!passed) {
		fprintf(stdout, "testHeapRefCallbackNoVisit, expected %d, found %d\n", 0, heapVisitControlFlags);
		fprintf(stdout, "testHeapRefCallbackNoVisit FAILED\n");
	}

	teardown();
	return passed;
}

bool testGetClassSignatureFailure() {
	setup();

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

    bool passed = false;
	try {
		MockGetClassSignatureReturnValue = JVMTI_ERROR_ACCESS_DENIED;
		MockGetLoadedClassesResultantClassCount = 2;
	    MockGetLoadedClassesResultantClasses = new jclass[2];
		heapHistogramAction->act(mockJNIEnv, 0);
    } catch (std::runtime_error *re) {
    	passed = (MockGetClassSignatureCount == 1);
	}

	if (!passed) {
		fprintf(stdout, "testGetClassSignatureFailure FAILED\n");
	}

	teardown();
	return passed;
}

bool testSetTagOnClassFailure() {
	setup();

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

    bool passed = false;
	try {
		MockSetTagReturnValue = JVMTI_ERROR_ACCESS_DENIED;
		MockGetLoadedClassesResultantClassCount = 2;
	    MockGetLoadedClassesResultantClasses = new jclass[2];
		heapHistogramAction->act(mockJNIEnv, 0);
    } catch (std::runtime_error *re) {
    	passed = (MockGetClassSignatureCount == 1);
	}

	if (!passed) {
		fprintf(stdout, "testSetTagOnClassFailure FAILED\n");
	}

	teardown();
	return passed;
}

jint followReferencesAction() {
	jlong classTag = MockSetTagTagsSet[0];
	jlong tag = 0;
	return (*MockHeapReferenceCallback)(JVMTI_HEAP_REFERENCE_CLASS,
										NULL,
										classTag,
										0,
										0,
										&tag,
										NULL,
										0,
										heapHistogramAction);
}

bool testHeapReferenceCallbackCallsRecordObject() {
	setup();

	heapHistogramAction = new HeapHistogramAction(mockJvmtiEnv, MockHSFactory);

	MockGetLoadedClassesResultantClassCount = 1;
	MockGetLoadedClassesResultantClasses = new jclass[1];
	MockGetClassSignatureResult = (char*)"test-signature";
	MockFollowReferencesAction = &followReferencesAction;

	heapHistogramAction->act(mockJNIEnv, 0);

    bool passed = (MockRecordObjectCount == 1);

	if (!passed) {
		fprintf(stdout, "testHeapReferenceCallbackCallsRecordObject FAILED\n");
	}

	teardown();
	return passed;
}

int main() {
	bool result = (testConstructionOk() &&
						testConstructionGetCapabilitiesFailure() &&
						testConstructionAddCapabilitiesFailure() &&
						testHeapStatsPrintOk() &&
						testFollowReferencesFailure() &&
						testHeapStatsTagsClassesReturnedByGetLoadedClasses() &&
						testHeapStatsGetLoadedClassesFailure() &&
						testHeapRefCallback() &&
						testHeapRefCallbackNoVisit() &&
						testGetClassSignatureFailure() &&
						testSetTagOnClassFailure() &&
						testHeapReferenceCallbackCallsRecordObject());
	if (result) {
       fprintf(stdout, "SUCCESS\n");
	   exit(EXIT_SUCCESS);
	}
	else {
    	fprintf(stdout, "FAILURE\n");
    	exit(EXIT_FAILURE);
	}
}
