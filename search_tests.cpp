#include <gtest/gtest.h>

#include "search.h"
#include "tests/build_index.h"
#include "tests/clean_token.h"
#include "tests/find_query_matches.h"
#include "tests/gather_tokens.h"

// TODO_STUDENT: write your tests here!

TEST(CleanToken, ExampleEmptyTest) {
    cout << "Hi! This shows up in the terminal." << endl;
}

TEST(CleanToken, RemovesPunctuation) {
    EXPECT_EQ(cleanToken("hello!"), "hello");
    EXPECT_EQ(cleanToken(",world"), "world");
}

TEST(CleanToken, ConvertsToLowercase) {
    EXPECT_EQ(cleanToken("Hello"), "hello");
    EXPECT_EQ(cleanToken("WORLD"), "world");
}

TEST(CleanToken, IgnoresNonAlphabetic) {
    EXPECT_EQ(cleanToken("12345"), "");
    EXPECT_EQ(cleanToken("!!!"), "");
}

TEST(CleanToken, HandlesEmptyString) {
    EXPECT_EQ(cleanToken(""), "");
}

TEST(CleanToken, HandlesPunctuationAtBothEnds) {
    EXPECT_EQ(cleanToken("!hello!"), "hello");
    EXPECT_EQ(cleanToken(",world,"), "world");
}

TEST(CleanToken, HandlesPunctuationInMiddle) {
    EXPECT_EQ(cleanToken("he,llo"), "he,llo");
    EXPECT_EQ(cleanToken("wo.rld"), "wo.rld");
}

TEST(CleanToken, HandlesPunctuationInMiddleAndStart) {
    EXPECT_EQ(cleanToken("!he,llo"), "he,llo");
    EXPECT_EQ(cleanToken(",wo.rld"), "wo.rld");
}

TEST(CleanToken, HandlesPunctuationInMiddleAndEnd) {
    EXPECT_EQ(cleanToken("he,llo!"), "he,llo");
    EXPECT_EQ(cleanToken("wo.rld,"), "wo.rld");
}

TEST(GatherTokens, SplitsWords) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("Hello world"), expected);
}

TEST(GatherTokens, RemovesPunctuation) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("hello, world!"), expected);
}

TEST(GatherTokens, ConvertsToLowercase) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("Hello WORLD"), expected);
}

TEST(GatherTokens, IgnoresNonAlphabetic) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("123 hello 456 world 789"), expected);
}

TEST(GatherTokens, HandlesSpacesAtBeginning) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("  Hello world"), expected);
}

TEST(GatherTokens, HandlesSpacesAtEnd) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("Hello world  "), expected);
}

TEST(GatherTokens, HandlesMultipleSpacesBetweenTokens) {
    set<string> expected = {"hello", "world"};
    EXPECT_EQ(gatherTokens("Hello   world"), expected);
}

TEST(BuildIndex, HandlesTinyFile) {
    map<string, set<string>> index;
    buildIndex("tiny.txt", index);

    // Expected tokens and corresponding URLs based on the content of tiny.txt
    EXPECT_EQ(index["eggs"], (set<string>{"www.shoppinglist.com"}));
    EXPECT_EQ(index["milk"], (set<string>{"www.shoppinglist.com"}));
    EXPECT_EQ(index["fish"], (set<string>{"www.shoppinglist.com", "www.dr.seuss.net"}));
    EXPECT_EQ(index["bread"], (set<string>{"www.shoppinglist.com"}));
    EXPECT_EQ(index["cheese"], (set<string>{"www.shoppinglist.com"}));
    EXPECT_EQ(index["red"], (set<string>{"www.rainbow.org", "www.dr.seuss.net"}));
    EXPECT_EQ(index["green"], (set<string>{"www.rainbow.org"}));
    EXPECT_EQ(index["orange"], (set<string>{"www.rainbow.org"}));
    EXPECT_EQ(index["yellow"], (set<string>{"www.rainbow.org"}));
    EXPECT_EQ(index["blue"], (set<string>{"www.rainbow.org", "www.dr.seuss.net"}));
    EXPECT_EQ(index["indigo"], (set<string>{"www.rainbow.org"}));
    EXPECT_EQ(index["violet"], (set<string>{"www.rainbow.org"}));
    EXPECT_EQ(index["one"], (set<string>{"www.dr.seuss.net"}));
    EXPECT_EQ(index["two"], (set<string>{"www.dr.seuss.net"}));
    EXPECT_EQ(index["not"], (set<string>{"www.bigbadwolf.com"}));
    EXPECT_EQ(index["trying"], (set<string>{"www.bigbadwolf.com"}));
    EXPECT_EQ(index["eat"], (set<string>{"www.bigbadwolf.com"}));
    EXPECT_EQ(index["you"], (set<string>{"www.bigbadwolf.com"}));

    // Ensure that non-existing tokens are not in the index
    EXPECT_TRUE(index.find("nonexistenttoken") == index.end());
}

TEST(BuildIndex, HandlesInvalidFilename) { 
  map<string, set<string>> results;
  int text = buildIndex("nonexistentfile.txt", results);

  EXPECT_EQ(text, 0);
}

TEST(FindQueryMatches, FindsSingleTermMatches) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "hello"), (set<string>{"url1.com", "url2.com"}));
}

TEST(FindQueryMatches, HandlesUnion) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "hello world"), (set<string>{"url1.com", "url2.com"}));
}

TEST(FindQueryMatches, HandlesIntersection) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "+hello +world"), (set<string>{"url2.com"}));
}

TEST(FindQueryMatches, HandlesDifference) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "hello -world"), (set<string>{"url1.com"}));
}

TEST(FindQueryMatches, FirstTermNotInMap) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_TRUE(findQueryMatches(index, "nonexistent").empty());
}

TEST(FindQueryMatches, PlusTermNotInMap) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_TRUE(findQueryMatches(index, "hello +nonexistent").empty());
}

TEST(FindQueryMatches, MinusTermNotInMap) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "hello -nonexistent"), (set<string>{"url1.com", "url2.com"}));
}


TEST(FindQueryMatches, UnmodifiedTermNotInMap) {
    map<string, set<string>> index = {{"hello", {"url1.com", "url2.com"}}, {"world", {"url2.com"}}};
    EXPECT_EQ(findQueryMatches(index, "hello nonexistent"), (set<string>{"url1.com", "url2.com"}));
}
