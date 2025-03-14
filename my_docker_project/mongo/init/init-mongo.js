db.createUser({
    user: "dbuser",
    pwd: "dbpasswd",
    roles: [{
        role: "readWrite",
        db: "taist_db"
    }]
});