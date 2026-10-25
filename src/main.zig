const std = @import("std");
const solver = @import("cryptarithmetic_puzzle");

pub fn main(init: std.process.Init) !void {
    const io = init.io;
    var stdout = std.Io.File.stdout().writer(io, &.{});
    // IDEA: implement an args parser
    // IDEA: handle x ammount of args e.g. a+b+c=d
    // IDEA: find all solutions via --all, otherwise default to the first one

    const a: []const u8 = "SEND";
    const b: []const u8 = "MORE";
    const c: []const u8 = "MONEY";
    if (solver.solve(a, b, c)) |sol| {
        try stdout.interface.print("{d} + {d} = {d}\n", .{ sol.a, sol.b, sol.sum });
    } else {
        try stdout.interface.print("No solution found\n", .{});
    }
    try stdout.flush();
}
