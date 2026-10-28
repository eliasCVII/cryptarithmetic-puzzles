const std = @import("std");

pub const Stats = struct {
    nodes_visites: usize,
    conflicts: usize,
    solution_count: usize,
    unique_chars: [10]u8 = undefined,
    unique_len: usize = 0,
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

pub fn char_to_digit(char: u8, unique_chars: []const u8, assigned_digits: []const u8) u8 {
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

pub fn word_to_number(word: []const u8, unique_chars: []const u8, assigned_digits: []const u8) u64 {
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

pub fn sort_chars(unique_chars: []u8, weights: []i64) void {
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
        }
    }
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
    const result_word = words[words.len - 1];
    if (words.len - 1 == 2 and result_word.len > max_len) { // with 2 addends, carry can only be 1
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

pub const Solver = struct {
    weights: []const i64,
    unique_chars: []const u8,
    domains: []const Domain,
    max_suffix: []const i64,
    min_suffix: []const i64,
    last_nz: usize,
    assigned_digits: *[10]u8,
    solutions: [][10]u8,
    stats: *Stats,

    fn search(self: *Solver, index: usize, current_sum: i64, used_mask: u16) void {
        self.stats.nodes_visites += 1;
        if (index == self.unique_chars.len) { // you assigned numbers to all the chars
            if (current_sum == 0) {
                if (self.stats.solution_count < self.solutions.len) {
                    @memcpy(&self.solutions[self.stats.solution_count], self.assigned_digits[0..10]);
                }
                self.stats.solution_count += 1;
            }
            return;
        }
        if (current_sum + self.max_suffix[index] < 0) {
            self.stats.conflicts += 1;
            return;
        }
        if (current_sum + self.min_suffix[index] > 0) {
            self.stats.conflicts += 1;
            return;
        }
        const w = self.weights[index];
        if (index == self.last_nz and w != 0) {
            if (@rem(-current_sum, w) == 0) {
                const d_i64 = @divTrunc(-current_sum, w);
                if (d_i64 >= 0 and d_i64 <= 9) {
                    const d: u4 = @intCast(d_i64);
                    if (self.domains[index].contains(d) and (used_mask & (@as(u16, 1) << d)) == 0) {
                        self.assigned_digits[index] = d;
                        self.search(index + 1, current_sum + w * d_i64, used_mask | (@as(u16, 1) << d));
                    }
                }
            }
            return;
        }
        for (0..10) |d| {
            if ((used_mask & @as(u16, 1) << @intCast(d)) != 0) continue;
            if (!self.domains[index].contains(d)) continue;
            self.assigned_digits[index] = @intCast(d);
            const next_sum = current_sum + self.weights[index] * @as(i64, @intCast(d));
            self.search(index + 1, next_sum, used_mask | (@as(u16, 1) << @intCast(d)));
        }
    }
};

pub fn solve(words: []const []const u8, solutions: [][10]u8) ?Stats {
    var stats: Stats = .{ .conflicts = 0, .nodes_visites = 0, .solution_count = 0 };
    var unique: UniqueChars = undefined;
    var factors: [16]i64 = undefined;
    var weights: [10]i64 = undefined;
    var domains: [10]Domain = @splat(Domain.all());
    var max_suffix: [10]i64 = @splat(0);
    var min_suffix: [10]i64 = @splat(0);
    var assigned_digits: [10]u8 = undefined;
    var last_nz: usize = 0;

    for (words, 0..) |word, i| {
        if (word.len == 0) return null;
        if (i < words.len - 1) {
            factors[i] = 1;
        } else {
            factors[words.len - 1] = -1;
        }
    }
    unique = get_unique_chars(words) catch return null;
    stats.unique_chars = unique.chars;
    stats.unique_len = unique.len;
    weights = compute_weights(words, unique.slice(), factors[0..words.len]);
    sort_chars(unique.slice(), &weights);
    for (weights[0..unique.len], 0..) |w, i| {
        if (w != 0) last_nz = i;
    }
    presolve(words, unique.slice(), &domains);
    precomp_branches(unique.slice(), &weights, &max_suffix, &min_suffix);
    var s = Solver{
        .weights = &weights,
        .unique_chars = unique.slice(),
        .domains = &domains,
        .max_suffix = &max_suffix,
        .min_suffix = &min_suffix,
        .last_nz = last_nz,
        .assigned_digits = &assigned_digits,
        .solutions = solutions,
        .stats = &stats
    };
    s.search(0, 0, 0);
    if (stats.solution_count > 0) return stats;
    return null;
}

fn benchSolve(name: []const u8, words: []const []const u8) ?Stats {
    var solutions: [50000][10]u8 = undefined;
    const start = std.Io.Timestamp.now(std.testing.io, .awake);
    const stats = solve(words, &solutions);
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

test "ABC + DEF = GHIJ (10 unique letters - all 96 solutions)" {
    const stats = benchSolve("ABC + DEF = GHIJ", &.{ "ABC", "DEF", "GHIJ" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 96), stats.?.solution_count);
}

test "SO + MANY = MORE (8 unique letters - all 748 solutions)" {
    const stats = benchSolve("SO + MANY = MORE", &.{ "SO", "MANY", "MORE" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 748), stats.?.solution_count);
}

test "AB + CD + EF = GHI (9 unique letters - all 2556 solutions)" {
    const stats = benchSolve("AB + CD + EF = GHI", &.{ "AB", "CD", "EF", "GHI" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 2556), stats.?.solution_count);
}

test "AB + CDE + FG = HIJ (10 unique letters - all 2184 solutions)" {
    const stats = benchSolve("AB + CDE + FG = HIJ", &.{ "AB", "CDE", "FG", "HIJ" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 2184), stats.?.solution_count);
}

test "AB + CD + EF + G = HIJ (10 unique letters - all 2160 solutions)" {
    const stats = benchSolve("AB + CD + EF + G = HIJ", &.{ "AB", "CD", "EF", "G", "HIJ" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 2160), stats.?.solution_count);
}

test "A + B + C + D + E = FG (7 unique letters - all 5640 solutions)" {
    const stats = benchSolve("A + B + C + D + E = FG", &.{ "A", "B", "C", "D", "E", "FG" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 5640), stats.?.solution_count);
}

test "A + B + C + D + E + F = GH (8 unique letters - all 16560 solutions)" {
    const stats = benchSolve("A + B + C + D + E + F = GH", &.{ "A", "B", "C", "D", "E", "F", "GH" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 16560), stats.?.solution_count);
}

test "A + B + C + D + E + F + G = HI (9 unique letters - all 35280 solutions)" {
    const stats = benchSolve("A + B + C + D + E + F + G = HI", &.{ "A", "B", "C", "D", "E", "F", "G", "HI" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 35280), stats.?.solution_count);
}

test "A + B + C + D + E + F + G + H = IJ (10 unique letters - all 40320 solutions)" {
    const stats = benchSolve("A + B + C + D + E + F + G + H = IJ", &.{ "A", "B", "C", "D", "E", "F", "G", "H", "IJ" });
    try std.testing.expect(stats != null);
    try std.testing.expectEqual(@as(usize, 40320), stats.?.solution_count);
}
