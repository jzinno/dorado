#include "TestUtils.h"
#include "htslib/sam.h"
#include "utils/bam_utils.h"

#include <catch2/catch.hpp>

#include <filesystem>

#define TEST_GROUP "[bam_utils][hts_writer]"

namespace fs = std::filesystem;
using namespace dorado::utils;

class HtsWriterTestsFixture {
public:
    HtsWriterTestsFixture() {
        fs::path aligner_test_dir = fs::path(get_data_dir("bam_reader"));
        m_in_sam = aligner_test_dir / "small.sam";
        m_out_bam = fs::temp_directory_path() / "out.bam";
    }

    ~HtsWriterTestsFixture() { fs::remove(m_out_bam); }

protected:
    void generate_bam(HtsWriter::OutputMode mode, int num_threads) {
        HtsReader reader(m_in_sam.string());
        HtsWriter writer(m_out_bam.string(), mode, num_threads, 0);

        writer.add_header(reader.header);
        writer.write_header();
        reader.read(writer, 1000);

        writer.join();
    }

private:
    fs::path m_in_sam;
    fs::path m_out_bam;
};

TEST_CASE_METHOD(HtsWriterTestsFixture, "HtsWriterTest: Write BAM", TEST_GROUP) {
    int num_threads = GENERATE(1, 10);
    HtsWriter::OutputMode emit_fastq = GENERATE(
            HtsWriter::OutputMode::SAM, HtsWriter::OutputMode::BAM, HtsWriter::OutputMode::FASTQ);
    REQUIRE_NOTHROW(generate_bam(emit_fastq, num_threads));
}

TEST_CASE("HtsWriterTest: Output mode conversion", TEST_GROUP) {
    CHECK(HtsWriter::get_output_mode("sam") == HtsWriter::OutputMode::SAM);
    CHECK(HtsWriter::get_output_mode("bam") == HtsWriter::OutputMode::BAM);
    CHECK(HtsWriter::get_output_mode("fastq") == HtsWriter::OutputMode::FASTQ);
    CHECK_THROWS_WITH(HtsWriter::get_output_mode("blah"), "Unknown output mode: blah");
}
