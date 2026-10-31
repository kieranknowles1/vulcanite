Integration tests are called from the [[Tool Scripts#`integrationtests`]] script and consist of a fixed sequence of inputs run against the final executable that should not cause any errors when executed.

Each keyframe of a test case defines the inputs that are active and a duration in frames. See existing tests for examples.

# Conventions
Tests should be padded on either side by frames with no inputs such that there will be frames in flight before and to test that the engine continues to function afterwards.