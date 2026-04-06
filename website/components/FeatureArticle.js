import PageBanner from "@/components/PageBanner";

const FeatureArticle = ({ title, category, slug, html }) => {
  return (
    <>
      <PageBanner
        pageName={title}
        pageLink={`/features/${slug}`}
        parentName="Features"
        parentLink="/features"
      />
      <div className="section blog">
        <div className="content">
          <div
            style={{
              textTransform: "uppercase",
              letterSpacing: "2px",
              fontSize: "0.7em",
              color: "#888",
              marginBottom: "24px",
            }}
          >
            {category}
          </div>
          <div
            className="single-post-text"
            dangerouslySetInnerHTML={{ __html: html }}
          />
          <div className="clear" />
        </div>
      </div>
    </>
  );
};

export default FeatureArticle;