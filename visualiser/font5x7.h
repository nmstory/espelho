#pragma once

#include <array>
#include <string_view>

namespace font5x7
{

using Glyph = std::array<std::string_view, 7>;

constexpr int glyphWidth = 5;
constexpr int glyphHeight = 7;

// Tiny 5x7 bitmap font covering just the characters the visualiser's HUD
// needs (digits, O/W/N/P/E/R/H/X/Y, and a handful of punctuation). '#' marks
// a lit pixel, anything else is background.
inline const Glyph& forChar(char c)
{
  static const Glyph digit0 = {
      ".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."};
  static const Glyph digit1 = {
      "..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."};
  static const Glyph digit2 = {
      ".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"};
  static const Glyph digit3 = {
      ".###.", "#...#", "....#", "..##.", "....#", "#...#", ".###."};
  static const Glyph digit4 = {
      "...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."};
  static const Glyph digit5 = {
      "#####", "#....", "#....", "####.", "....#", "#...#", ".###."};
  static const Glyph digit6 = {
      "..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###."};
  static const Glyph digit7 = {
      "#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."};
  static const Glyph digit8 = {
      ".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."};
  static const Glyph digit9 = {
      ".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##.."};
  static const Glyph letterO = {
      ".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."};
  static const Glyph letterW = {
      "#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"};
  static const Glyph letterN = {
      "#...#", "##..#", "##..#", "#.#.#", "#..##", "#..##", "#...#"};
  static const Glyph letterP = {
      "####.", "#...#", "#...#", "####.", "#....", "#....", "#...."};
  static const Glyph letterE = {
      "#####", "#....", "#....", "####.", "#....", "#....", "#####"};
  static const Glyph letterR = {
      "####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"};
  static const Glyph letterH = {
      "#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"};
  static const Glyph letterX = {
      "#...#", ".#.#.", "..#..", "..#..", "..#..", ".#.#.", "#...#"};
  static const Glyph letterY = {
      "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#..", "..#.."};
  static const Glyph hash = {
      ".#.#.", ".#.#.", "#####", ".#.#.", "#####", ".#.#.", ".#.#."};
  static const Glyph colon = {
      ".....", "..#..", ".....", ".....", ".....", "..#..", "....."};
  static const Glyph dash = {
      ".....", ".....", ".....", "#####", ".....", ".....", "....."};
  static const Glyph space = {
      ".....", ".....", ".....", ".....", ".....", ".....", "....."};

  switch (c) {
    case '0':
      return digit0;
    case '1':
      return digit1;
    case '2':
      return digit2;
    case '3':
      return digit3;
    case '4':
      return digit4;
    case '5':
      return digit5;
    case '6':
      return digit6;
    case '7':
      return digit7;
    case '8':
      return digit8;
    case '9':
      return digit9;
    case 'O':
      return letterO;
    case 'W':
      return letterW;
    case 'N':
      return letterN;
    case 'P':
      return letterP;
    case 'E':
      return letterE;
    case 'R':
      return letterR;
    case 'H':
      return letterH;
    case 'X':
      return letterX;
    case 'Y':
      return letterY;
    case '#':
      return hash;
    case ':':
      return colon;
    case '-':
      return dash;
    default:
      return space;
  }
}

}  // namespace font5x7
