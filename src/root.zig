const std = @import("std");

pub const Solution = struct {
    a: u64,
    b: u64,
    sum: u64,
};

pub const Stats = struct {
    nodes_visites: usize,
    conflicts: usize,
    solution_count: usize
};

pub const Domain = struct {
    mask: u10,

    pub fn all() Domain {
        return .{ .mask = 0b11_1111_1111 };
    }

    pub fn remove(self: *Domain, digit: u4) void {
        self.mask = self.mask & ~(@as(u10, 1) << digit);
    }

    pub fn fix(self: *Domain, digit: u4) void {
        // set a fixed number
        self.mask = (@as(u10, 1) << digit);
    }

    pub fn is_fixed(self: Domain) bool {
        return @popCount(self.mask) == 1;
    }

    pub fn get_fixed(self: Domain) u4 {
        return @truncate(@ctz(self.mask));
    }

    pub fn contains(self: Domain, digit: usize) bool {
        return (self.mask & (@as(u10, 1) << @intCast(digit)) != 0);
    }
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

pub fn presolve(words: []const []const u8, unique_chars: []const u8, domains: []Domain) void {
    var max_len: usize = 0;

    for (words, 0..) |word, i| {
        if (word.len > 1) {
            const letter_pos = find_index(word[0], unique_chars);
            domains[letter_pos].remove(0);
        }
        if (i < words.len - 1) {
            max_len = @max(max_len, word.len);
        }
    }
    // for (words[0 .. words.len - 1]) |word| {
    //     max_len = @max(max_len, word.len);
    // }

    const result_word = words[words.len - 1];
    if (result_word.len > max_len) { // if c.len > a.len + b.len
        const target_ch = find_index(result_word[0], unique_chars);
        domains[target_ch].fix(1);
    }
    for (domains, 0..) |domain, k| {
        if (domain.is_fixed()) {
            const fixed_digit = domain.get_fixed();
            // do something here
            for (domains, 0..) |*d, j| {
                if (j != k) {
                    d.remove(fixed_digit);
                }
            }
        }
    }
}

pub fn assign_digits_cp(
    weights: []const i64,
    unique_chars: []const u8,
    assigned_digits: []u8,
    domains: []Domain,
    max_suffix: []i64,
    min_suffix: []i64,
    index: u8,
    current_sum: i64,
    used_mask: u16,
    stats: *Stats
) void {
    stats.nodes_visites += 1;
    if (index == unique_chars.len) { // you assigned numbers to all the chars
        if (current_sum == 0) {
            stats.solution_count += 1;
        }
        return;
    }
    if (current_sum + max_suffix[index] < 0) {
        stats.conflicts += 1;
        return;
    }
    if (current_sum + min_suffix[index] > 0) {
        stats.conflicts += 1;
        return;
    }
    for (0..10) |d| {
        if ((used_mask & @as(u16, 1) << @intCast(d)) != 0) continue;
        if (!domains[index].contains(d)) continue;
        assigned_digits[index] = @intCast(d);
        const next_sum = current_sum + weights[index] * @as(i64, @intCast(d));
        assign_digits_cp(
            weights,
            unique_chars,
            assigned_digits,
            domains,
            max_suffix,
            min_suffix,
            index + 1,
            next_sum,
            used_mask | (@as(u16, 1) << @intCast(d)),
            stats
        );
    }
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

pub fn solve_cp(words: []const []const u8) ?Stats {
    var factors: [16]i64 = undefined;
    for (words, 0..) |word, i| {
        if (word.len == 0) return null;
        if (i < words.len - 1) {
            factors[i] = 1;
        } else {
            factors[words.len-1] = -1;
        }
    }
    var unique = get_unique_chars(words) catch return null;
    var weights: [10]i64 = compute_weights(words, unique.slice(), factors[0..words.len]);

    var bit_mask: [10]bool = init_vars(words, unique.slice());
    var stats: Stats = .{ .conflicts = 0, .nodes_visites = 0, .solution_count = 0};
    sort_chars(unique.slice(), &weights, &bit_mask);
    var domains: [10]Domain = @splat(Domain.all());
    presolve(words, unique.slice(), &domains);
    var max_suffix: [10]i64 = @splat(0);
    var min_suffix: [10]i64 = @splat(0);
    precomp_branches(unique.slice(), &weights, &max_suffix, &min_suffix);

    var assigned_digits: [10]u8 = undefined;
    assign_digits_cp(
        &weights,
        unique.slice(),
        &assigned_digits,
        &domains,
        &max_suffix,
        &min_suffix,
        0,
        0,
        0,
        &stats,
    );

    if (stats.solution_count > 0) {
        return stats;
    }
    return null;
}

// ==========================================
// TEST SUITE
// ==========================================

fn benchSolve(name: []const u8, words: []const []const u8) ?Stats {
    const start = std.Io.Timestamp.now(std.testing.io, .awake);
    const stats = solve_cp(words);
    const end = std.Io.Timestamp.now(std.testing.io, .awake);
    const ns: f64 = @floatFromInt(end.nanoseconds - start.nanoseconds);
    const us = ns / 1000.0;
    const s = ns / 1_000_000_000.0;

    std.debug.print("\n  >> {s}: {d:.6} s ({d:.2} us)", .{ name, s, us });
    if (stats) |st| {
        std.debug.print(" (solutions: {d}, nodes: {d}, conflicts: {d})\n", .{ st.solution_count, st.nodes_visites, st.conflicts });
    } else {
        std.debug.print(" (no solution)\n", .{});
    }
    return stats;
}

test "SEND + MORE = MONEY" {
    const stats = benchSolve("SEND + MORE = MONEY", &.{ "SEND", "MORE", "MONEY" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 1), stats.?.solution_count);
}

test "BASE + BALL = GAMES" {
    const stats = benchSolve("BASE + BALL = GAMES", &.{ "BASE", "BALL", "GAMES" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 1), stats.?.solution_count);
}

test "CROSS + ROADS = DANGER" {
    const stats = benchSolve("CROSS + ROADS = DANGER", &.{ "CROSS", "ROADS", "DANGER" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 1), stats.?.solution_count);
}

test "Impossible puzzle: NO + NO = NO" {
    const stats = benchSolve("NO + NO = NO", &.{ "NO", "NO", "NO" });
    try std.testing.expect(stats == null);
}

test "Too many unique letters (> 10 letters)" {
    const stats = benchSolve("ABCDE + FGHIJ = KLM", &.{ "ABCDE", "FGHIJ", "KLM" });
    try std.testing.expect(stats == null);
}

test "CP + IS + FUN = TRUE (Google OR-Tools benchmark - all 72 solutions)" {
    const stats = benchSolve("CP + IS + FUN = TRUE", &.{ "CP", "IS", "FUN", "TRUE" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 72), stats.?.solution_count);
}
