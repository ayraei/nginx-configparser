#include "gtest/gtest.h"
#include "config_parser.h"
#include <sstream>
#include <string>

class NginxStringConfigTest : public ::testing::Test {
 protected:
  bool ParseString(const std::string& config_string) {
    std::stringstream config_stream(config_string);
    return parser_.Parse(&config_stream, &out_config_);
  }
  NginxConfigParser parser_;
  NginxConfig out_config_;
};

// Given
TEST_F(NginxStringConfigTest, SimpleConfig) {
  EXPECT_TRUE(ParseString("foo bar;"));
  // We can also add expectations about the contents of the statements:
  EXPECT_EQ(1, out_config_.statements_.size());
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
}

// Given
TEST(NginxConfigTest, ToStringSimple) {
  NginxConfigStatement statement;
  statement.tokens_.push_back("foo");
  statement.tokens_.push_back("bar");
  EXPECT_EQ("foo bar;\n", statement.ToString(0));
}

// This test fails before the parser bugfix
TEST_F(NginxStringConfigTest, SimpleBadConfig) {
  EXPECT_FALSE(ParseString("foo bar"));
  EXPECT_FALSE(ParseString(";"));
  EXPECT_FALSE(ParseString("incomplete { foo bar;"));
  EXPECT_FALSE(ParseString("foo {"));
  EXPECT_FALSE(ParseString("foo; }"));
  EXPECT_FALSE(ParseString("}"));
  EXPECT_FALSE(ParseString("{ abc xyz; }"));
}

// This test fails before parser bugfix
TEST_F(NginxStringConfigTest, NestedBlock) {
  EXPECT_TRUE(ParseString("foo { bar { abc xyz;\n123 456; } }"));
  EXPECT_EQ(1, out_config_.statements_.size());
  
  // Check multiple nested blocks get tokenized correctly
  NginxConfig child = *out_config_.statements_.at(0)->child_block_;
  EXPECT_EQ("bar", child.statements_.at(0)->tokens_.at(0));
  child = *child.statements_.at(0)->child_block_;
  EXPECT_EQ("abc", child.statements_.at(0)->tokens_.at(0));
}

TEST_F(NginxStringConfigTest, TwoLineConfig) {
  EXPECT_TRUE(ParseString("foo bar;\nabc 123;"));
  EXPECT_EQ(2, out_config_.statements_.size());
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
  EXPECT_EQ("123", out_config_.statements_.at(1)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, Comment) {
  EXPECT_TRUE(ParseString("# Oh look a comment\nfoo bar;\n# And another!"));
  EXPECT_EQ("bar", out_config_.statements_.at(0)->tokens_.at(1));
}
