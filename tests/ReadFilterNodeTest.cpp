#include "read_pipeline/ReadFilterNode.h"

#include "MessageSinkUtils.h"

#include <catch2/catch.hpp>

#define TEST_GROUP "[read_pipeline][ReadFilterNode]"

TEST_CASE("ReadFilterNode: Filter read based on qscore", TEST_GROUP) {
    MessageSinkToVector<std::shared_ptr<dorado::Read>> sink(100);
    {
        std::shared_ptr<dorado::Read> read_1(new dorado::Read());
        read_1->raw_data = torch::empty(100);
        read_1->sample_rate = 4000;
        read_1->shift = 128.3842f;
        read_1->scale = 8.258f;
        read_1->read_id = "read_1";
        read_1->seq = "ACGTACGT";
        read_1->qstring = "********";  // average q score 9
        read_1->num_trimmed_samples = 132;
        read_1->attributes.mux = 2;
        read_1->attributes.read_number = 18501;
        read_1->attributes.channel_number = 5;
        read_1->attributes.start_time = "2017-04-29T09:10:04Z";
        read_1->attributes.fast5_filename = "batch_0.fast5";

        std::shared_ptr<dorado::Read> read_2(new dorado::Read());
        read_2->raw_data = torch::empty(100);
        read_2->sample_rate = 4000;
        read_2->shift = 128.3842f;
        read_2->scale = 8.258f;
        read_2->read_id = "read_2";
        read_2->seq = "ACGTACGT";
        read_2->qstring = "////////";  // average q score 14
        read_2->num_trimmed_samples = 132;
        read_2->attributes.mux = 2;
        read_2->attributes.read_number = 18501;
        read_2->attributes.channel_number = 5;
        read_2->attributes.start_time = "2017-04-29T09:10:04Z";
        read_2->attributes.fast5_filename = "batch_0.fast5";

        dorado::ReadFilterNode filter(sink, 12 /*min_qscore*/, 0, 2 /*threads*/);
        filter.push_message(read_1);
        filter.push_message(read_2);
    }

    auto messages = sink.get_messages();
    REQUIRE(messages.size() == 1);
}
