const std = @import("std");

pub const Solution = struct {
    a: u64,
    b: u64,
    sum: u64,
};

pub const UniqueChars = struct {
    chars: [10]u8,
    len: usize,

    pub fn slice(self: *UniqueChars) []u8 {
        return self.chars[0..self.len];
    }
};

pub fn get_unique_chars(words: []const []const u8) !UniqueChars {
    var result: UniqueChars = .{ .chars = undefined, .len = 0 };
    var seen: u32 = 0;

    for (words) |word| {
        for (word) |char| {
            const bit = @as(u32, 1) << @intCast(char - 'A');
            if ((seen & bit) == 0) {
                seen |= bit;
                if (result.len >= 10) return error.TooManyUniqueChars;
                result.chars[result.len] = char;
                result.len += 1;
            }
        }
    }
    return result;
}

pub fn char_to_digit(char: u8, unique_chars: []const u8, assigned_digits: []u8) u8 {
    // getter, returns the value in assigned_digits related to unique_chars
    for (unique_chars, 0..) |c, i| {
        if (c == char) return assigned_digits[i];
    }
    unreachable;
}

pub fn find_index(char: u8, unique_chars: []const u8) usize {
    for (unique_chars, 0..) |c, i| {
        if (c == char) return i;
    }
    unreachable;
}

pub fn word_to_number(word: []const u8, unique_chars: []const u8, assigned_digits: []u8) u64 {
    // all the words to a number, once they have been assigned?
    var result: u64 = 0;
    for (word) |char| {
        const digit: u8 = char_to_digit(char, unique_chars, assigned_digits);
        result = result * 10 + digit;
    }
    return result;
}

pub fn compute_weights(words: []const []const u8, unique_chars: []const u8, factor: []const i64) [10]i64 {
    var weights: [10]i64 = @splat(0);
    for (words, factor) |word, f| {
        var m: i64 = f;
        for (0..word.len) |i| {
            const ch: u8 = word[word.len - i - 1];
            weights[find_index(ch, unique_chars)] += m;
            m *= 10;
        }
    }
    return weights;
}

pub fn precomp_branches(
    unique_chars: []const u8,
    weights: []const i64,
    max_suffix: []i64,
    min_suffix: []i64,
) void {
    var pos_sum: i64 = 0;
    var neg_sum: i64 = 0;
    var i: usize = unique_chars.len;

    while (i > 0) {
        i -= 1;
        const w = weights[i];
        if (w > 0) {
            pos_sum += w * 9;
        } else if (w < 0) {
            neg_sum += w * 9;
        }
        max_suffix[i] = pos_sum;
        min_suffix[i] = neg_sum;
    }
}

pub fn init_vars(words: []const []const u8, unique_chars: []const u8) [10]bool {
    var bit_mask: [10]bool = @splat(false);
    for (words) |word| {
        if (word.len == 1) continue;
        bit_mask[find_index(word[0], unique_chars)] = true;
    }
    return bit_mask;
}

pub fn sort_chars(unique_chars: []u8, weights: []i64, bit_mask: []bool) void {
    for (0..unique_chars.len) |i| {
        var tope = i;
        for (i + 1..unique_chars.len) |j| {
            if (@abs(weights[j]) > @abs(weights[tope])) {
                tope = j;
            }
        }
        if (tope != i) {
            std.mem.swap(i64, &weights[i], &weights[tope]);
            std.mem.swap(u8, &unique_chars[i], &unique_chars[tope]);
            std.mem.swap(bool, &bit_mask[i], &bit_mask[tope]);
        }
    }
}

pub fn assign_digits(weights: []const i64, unique_chars: []const u8, assigned_digits: []u8, bit_mask: []const bool, max_suffix: []i64, min_suffix: []i64, index: u8, current_sum: i64, used_mask: u16) bool {
    // go through every char in unique chars, look for a number between 0-9
    // and then set it in a, b and c
    // backtrack if it returns false and try a new number

    // Base case
    if (index == unique_chars.len) { // you assigned numbers to all the chars
        return current_sum == 0;
    }
    if (current_sum + max_suffix[index] < 0) return false;
    if (current_sum + min_suffix[index] > 0) return false;
    for (0..10) |d| {
        if ((used_mask & @as(u16, 1) << @intCast(d)) != 0) continue;
        if (bit_mask[index] and d == 0) continue;
        assigned_digits[index] = @intCast(d);
        const next_sum = current_sum + weights[index] * @as(i64, @intCast(d));
        if (assign_digits(weights, unique_chars, assigned_digits, bit_mask, max_suffix, min_suffix, index + 1, next_sum, used_mask | (@as(u16, 1) << @intCast(d)))) {
            return true;
        }
    }
    return false;
}


pub fn solve(a: []const u8, b: []const u8, c: []const u8) ?Solution {
    if (a.len == 0 or b.len == 0 or c.len == 0) return null;
    var unique = get_unique_chars(&.{ a, b, c }) catch return null;
    var weights: [10]i64 = compute_weights(&.{ a, b, c }, unique.slice(), &.{ 1, 1, -1 });
    var bit_mask: [10]bool = init_vars(&.{ a, b, c }, unique.slice());
    sort_chars(unique.slice(), &weights, &bit_mask);

    var max_suffix: [10]i64 = @splat(0);
    var min_suffix: [10]i64 = @splat(0);
    precomp_branches(unique.slice(), &weights, &max_suffix, &min_suffix);

    var assigned_digits: [10]u8 = undefined;
    if (assign_digits(
        &weights,
        unique.slice(),
        &assigned_digits,
        &bit_mask,
        &max_suffix,
        &min_suffix,
        0,
        0,
        0,
    )) {
        return .{
            .a = word_to_number(a, unique.slice(), &assigned_digits),
            .b = word_to_number(b, unique.slice(), &assigned_digits),
            .sum = word_to_number(c, unique.slice(), &assigned_digits),
        };
    }
    return null;
}

// ==========================================
// TEST SUITE
// ==========================================

test "SEND + MORE = MONEY" {
    const sol = solve("SEND", "MORE", "MONEY");
    try std.testing.expect(sol != null);
    try std.testing.expectEqual(@as(u64, 9567), sol.?.a);
    try std.testing.expectEqual(@as(u64, 1085), sol.?.b);
    try std.testing.expectEqual(@as(u64, 10652), sol.?.sum);
}

test "BASE + BALL = GAMES" {
    const sol = solve("BASE", "BALL", "GAMES");
    try std.testing.expect(sol != null);
    try std.testing.expectEqual(@as(u64, 7483), sol.?.a);
    try std.testing.expectEqual(@as(u64, 7455), sol.?.b);
    try std.testing.expectEqual(@as(u64, 14938), sol.?.sum);
}

test "CROSS + ROADS = DANGER" {
    const sol = solve("CROSS", "ROADS", "DANGER");
    try std.testing.expect(sol != null);
    try std.testing.expectEqual(@as(u64, 96233), sol.?.a);
    try std.testing.expectEqual(@as(u64, 62513), sol.?.b);
    try std.testing.expectEqual(@as(u64, 158746), sol.?.sum);
}

test "Impossible puzzle: NO + NO = NO" {
    const sol = solve("NO", "NO", "NO");
    try std.testing.expect(sol == null);
}

test "Too many unique letters (> 10 letters)" {
    const sol = solve("ABCDE", "FGHIJ", "KLM");
    try std.testing.expect(sol == null);
}
